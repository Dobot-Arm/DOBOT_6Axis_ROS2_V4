#include <dobot_bringup/tcp_socket.h>
#include <rclcpp/rclcpp.hpp>
#include <netinet/tcp.h>

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
        ::close(fd_);
    fd_ = -1;
    is_connected_ = false;
}

void TcpClient::connect()
{
    // Always start from a fresh socket. A TCP socket whose ::connect() failed is
    // spent: further attempts on the same fd fail permanently (EINVAL /
    // ECONNABORTED), so reusing it wedged the reconnect loop for the lifetime of
    // the process once a single attempt had missed — which is exactly what
    // happens while the controller is rebooting.
    close();

    fd_ = ::socket(AF_INET, SOCK_STREAM, 0);
    if (fd_ < 0)
        throw TcpClientException(toString() + std::string(" socket : ") + strerror(errno));

#ifdef TCP_SYNCNT
    // Bound the SYN retries so an attempt against a powered-off controller fails
    // in seconds rather than the ~2 min kernel default, keeping the retry cadence
    // (and therefore recovery latency) predictable.
    int syn_retries = 2;
    ::setsockopt(fd_, IPPROTO_TCP, TCP_SYNCNT, &syn_retries, sizeof(syn_retries));
#endif

    sockaddr_in addr = {};

    memset(&addr, 0, sizeof(addr));
    inet_pton(AF_INET, ip_.c_str(), &addr.sin_addr);
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port_);

    if (::connect(fd_, (sockaddr *)&addr, sizeof(addr)) < 0)
    {
        std::string what = toString() + std::string(" connect : ") + strerror(errno);
        close();
        throw TcpClientException(what);
    }
    is_connected_ = true;

    RCLCPP_INFO(rclcpp::get_logger("dobot_tcp"), "connected to %s", toString().c_str());
}

void TcpClient::disConnect()
{
    // Previously cleared fd_ *before* ::close(fd_), so it closed -1 and leaked the
    // real descriptor on every disconnect.
    close();
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
        ssize_t bytes_sent = ::send(fd_, tmp, len, MSG_NOSIGNAL);
        if (bytes_sent < 0)
        {
            disConnect();
            throw TcpClientException(toString() + std::string(" ::send() ") + strerror(errno));
        }
        len -= bytes_sent;
        tmp += bytes_sent;
    }
}

bool TcpClient::tcpRecv(void *buf, uint32_t len, uint32_t &has_read, uint32_t timeout)
{
    uint8_t *tmp = (uint8_t *)buf; // NOLINT(modernize-use-auto)
    fd_set read_fds;
    timeval tv = {0, 0};

    has_read = 0;
    while (len)
    {
        FD_ZERO(&read_fds);
        FD_SET(fd_, &read_fds);

        tv.tv_sec = timeout / 1000;
        tv.tv_usec = (timeout % 1000) * 1000;
        int select_result = ::select(fd_ + 1, &read_fds, nullptr, nullptr, &tv);
        if (select_result < 0)
        {
            disConnect();
            throw TcpClientException(toString() + std::string(" select() : ") + strerror(errno));
        }
        else if (select_result == 0)
        {
            return false;
        }

        ssize_t bytes_read = ::read(fd_, tmp, len);
        if (bytes_read < 0)
        {
            disConnect();
            throw TcpClientException(toString() + std::string(" ::read() ") + strerror(errno));
        }
        else if (bytes_read == 0)
        {
            disConnect();
            throw TcpClientException(toString() + std::string(" tcp server has disconnected"));
        }
        has_read += bytes_read;
        tmp += bytes_read;
        len -= bytes_read;

        if (*(tmp - 1) == ';')
        {
            return true;
        }
    }
    return true;
}

bool TcpClient::tcpRecvExact(void *buf, uint32_t len, uint32_t timeout)
{
    uint8_t *ptr = static_cast<uint8_t *>(buf);
    uint32_t remaining = len;

    while (remaining > 0)
    {
        fd_set read_fds;
        FD_ZERO(&read_fds);
        FD_SET(fd_, &read_fds);

        timeval tv;
        tv.tv_sec = timeout / 1000;
        tv.tv_usec = (timeout % 1000) * 1000;

        int select_result = ::select(fd_ + 1, &read_fds, nullptr, nullptr, &tv);
        if (select_result < 0)
        {
            disConnect();
            throw TcpClientException(toString() + std::string(" select() : ") + strerror(errno));
        }
        else if (select_result == 0)
        {
            return false;
        }

        ssize_t bytes_read = ::read(fd_, ptr, remaining);
        if (bytes_read < 0)
        {
            disConnect();
            throw TcpClientException(toString() + std::string(" ::read() ") + strerror(errno));
        }
        else if (bytes_read == 0)
        {
            disConnect();
            throw TcpClientException(toString() + std::string(" tcp server has disconnected"));
        }

        ptr += bytes_read;
        remaining -= bytes_read;
    }

    return true;
}

std::string TcpClient::toString()
{
    return ip_ + ":" + std::to_string(port_);
}
