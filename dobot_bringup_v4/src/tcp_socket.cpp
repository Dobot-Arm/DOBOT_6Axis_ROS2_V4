#include <dobot_bringup/tcp_socket.h>
#include <cstring>

TcpClient::TcpClient(std::string ip, uint16_t port) : fd_(-1), port_(port), ip_(std::move(ip)), is_connected_(false)
{
}

TcpClient::~TcpClient()
{
    close();
}

void TcpClient::close()
{
    if (fd_ >= 0)
    {
        ::close(fd_);
        is_connected_ = false;
        fd_ = -1;
    }
}

void TcpClient::connect()
{
    if (fd_ < 0)
    {
        fd_ = ::socket(AF_INET, SOCK_STREAM, 0);
        if (fd_ < 0)
            throw TcpClientException(toString() + std::string(" socket : ") + strerror(errno));
    }

    sockaddr_in addr = {};

    memset(&addr, 0, sizeof(addr));
    inet_pton(AF_INET, ip_.c_str(), &addr.sin_addr);
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port_);

    if (::connect(fd_, (sockaddr *)&addr, sizeof(addr)) < 0)
        throw TcpClientException(toString() + std::string(" connect : ") + strerror(errno));
    is_connected_ = true;

    RCLCPP_INFO(rclcpp::get_logger("TcpClient"), "connect successfully: %s", toString().c_str());
}

void TcpClient::disConnect()
{
    if (is_connected_)
    {
        is_connected_ = false;
        ::close(fd_);
        fd_ = -1;
    }
    recv_buffer_.clear();
}

bool TcpClient::isConnect() const
{
    return is_connected_;
}

void TcpClient::tcpSend(const void *buf, uint32_t len)
{
    if (!is_connected_)
        throw TcpClientException("tcp is disconnected");

    const auto *tmp = (const uint8_t *)buf;
    while (len)
    {
        int err = (int)::send(fd_, tmp, len, MSG_NOSIGNAL);
        if (err < 0)
        {
            disConnect();
            throw TcpClientException(toString() + std::string(" ::send() ") + strerror(errno));
        }
        len -= err;
        tmp += err;
    }
}

bool TcpClient::tcpRecv(void *buf, uint32_t len, uint32_t &has_read, uint32_t timeout)
{
    uint8_t *tmp = (uint8_t *)buf;
    fd_set read_fds;
    timeval tv = {0, 0};

    has_read = 0;
    while (len > 0)
    {
        FD_ZERO(&read_fds);
        FD_SET(fd_, &read_fds);

        tv.tv_sec = timeout / 1000;
        tv.tv_usec = (timeout % 1000) * 1000;
        int err = ::select(fd_ + 1, &read_fds, nullptr, nullptr, &tv);
        if (err < 0)
        {
            disConnect();
            throw TcpClientException(toString() + std::string(" select() : ") + strerror(errno));
        }
        else if (err == 0)
        {
            return false;
        }

        err = (int)::read(fd_, tmp, len);
        if (err < 0)
        {
            disConnect();
            throw TcpClientException(toString() + std::string(" ::read() ") + strerror(errno));
        }
        else if (err == 0)
        {
            disConnect();
            throw TcpClientException(toString() + std::string(" tcp server has disconnected"));
        }

        has_read += err;
        
        for (int i = 0; i < err; ++i)
        {
            if (tmp[i] == ';')
            {
                return true;
            }
        }

        len -= err;
        tmp += err;
    }
    return true;
}


bool TcpClient::tcpRecvFrame(
    void *buf,
    uint32_t frame_len,
    uint32_t test_value_offset,
    uint64_t expected_test_value,
    uint32_t timeout)
{
    constexpr size_t CHUNK_SIZE = 4096;
    uint8_t chunk[CHUNK_SIZE];

    while (true)
    {
        // Search the accumulated TCP stream for a complete valid frame.
        if (recv_buffer_.size() >= frame_len)
        {
            for (size_t offset = 0;
                 offset + frame_len <= recv_buffer_.size();
                 ++offset)
            {
                uint64_t len_field = 0;
                uint64_t test_field = 0;

                memcpy(
                    &len_field,
                    recv_buffer_.data() + offset,
                    sizeof(len_field));

                memcpy(
                    &test_field,
                    recv_buffer_.data() + offset + test_value_offset,
                    sizeof(test_field));

                if (len_field == frame_len &&
                    test_field == expected_test_value)
                {
                    memcpy(
                        buf,
                        recv_buffer_.data() + offset,
                        frame_len);

                    // Discard everything up to and including this frame.
                    recv_buffer_.erase(
                        recv_buffer_.begin(),
                        recv_buffer_.begin() + offset + frame_len);

                    return true;
                }
            }

            // No complete valid frame was found.
            //
            // Keep the trailing bytes because they may contain the beginning
            // of a frame whose remainder has not arrived yet.
            if (recv_buffer_.size() > frame_len - 1)
            {
                recv_buffer_.erase(
                    recv_buffer_.begin(),
                    recv_buffer_.end() - (frame_len - 1));
            }
        }

        fd_set read_fds;
        FD_ZERO(&read_fds);
        FD_SET(fd_, &read_fds);

        timeval tv = {0, 0};
        tv.tv_sec = timeout / 1000;
        tv.tv_usec = (timeout % 1000) * 1000;

        int err = ::select(fd_ + 1, &read_fds, nullptr, nullptr, &tv);

        if (err < 0)
        {
            disConnect();
            throw TcpClientException(
                toString() + std::string(" select() : ") + strerror(errno));
        }
        else if (err == 0)
        {
            // Keep recv_buffer_ intact. A partial frame may simply be
            // completed on the next call.
            return false;
        }

        err = (int)::read(fd_, chunk, sizeof(chunk));

        if (err < 0)
        {
            disConnect();
            throw TcpClientException(
                toString() + std::string(" ::read() ") + strerror(errno));
        }
        else if (err == 0)
        {
            disConnect();
            throw TcpClientException(
                toString() + std::string(" tcp server has disconnected"));
        }

        recv_buffer_.insert(
            recv_buffer_.end(),
            chunk,
            chunk + err);
    }
}

std::string TcpClient::toString()
{
    return ip_ + ":" + std::to_string(port_);
}
