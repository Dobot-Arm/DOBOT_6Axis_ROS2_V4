#include <dobot_bringup/command.h>
#include <rclcpp/rclcpp.hpp>
#include <chrono>
#include <thread>

static const rclcpp::Logger kLogger = rclcpp::get_logger("dobot_tcp");
static constexpr uint32_t kRecvBufSize = 1024;

CRCommanderRos2::CRCommanderRos2(const std::string &ip)
    : current_joint_{}, tool_vector_{}, is_running_(false)
{
    is_running_ = false;
    real_time_data_ = std::make_shared<RealTimeData>();
    real_time_tcp_ = std::make_shared<TcpClient>(ip, 30004);
    dash_board_tcp_ = std::make_shared<TcpClient>(ip, 29999);
}

CRCommanderRos2::~CRCommanderRos2()
{
    is_running_ = false;
    thread_->join();
}

void CRCommanderRos2::getCurrentJointStatus(double *joint)
{
    mutex_.lock();
    memcpy(joint, current_joint_, sizeof(current_joint_));
    mutex_.unlock();
}

void CRCommanderRos2::getToolVectorActual(double *val)
{
    mutex_.lock();
    memcpy(val, tool_vector_, sizeof(tool_vector_));
    mutex_.unlock();
}

void CRCommanderRos2::recvTask()
{
    uint32_t has_read;
    while (is_running_)
    {
        if (real_time_tcp_->isConnect())
        {
            try
            {
                uint8_t *tmpData = reinterpret_cast<uint8_t *>(real_time_data_.get());
                if (real_time_tcp_->tcpRecv(tmpData, sizeof(RealTimeData), has_read, 5000))
                {

                    if (real_time_data_->len != 1440)
                        continue;

                    mutex_.lock();
                    for (uint32_t i = 0; i < 6; i++)
                        current_joint_[i] = deg2Rad(real_time_data_->q_actual[i]);

                    memcpy(tool_vector_, real_time_data_->tool_vector_actual, sizeof(tool_vector_));
                    mutex_.unlock();
                }
                else
                {
                    RCLCPP_WARN(kLogger, "tcp recv timeout");
                }
            }
            catch (const TcpClientException &err)
            {
                real_time_tcp_->disConnect();
                RCLCPP_ERROR(kLogger, "tcp recv error: %s", err.what());
            }
        }
        else
        {
            try
            {
                real_time_tcp_->connect();
            }
            catch (const TcpClientException &err)
            {
                RCLCPP_ERROR(kLogger, "realtime tcp connect failed: %s", err.what());
                sleep(3);
            }
        }

        if (!dash_board_tcp_->isConnect())
        {
            try
            {
                dash_board_tcp_->connect();
            }
            catch (const TcpClientException &err)
            {

                RCLCPP_ERROR(kLogger, "dashboard tcp connect failed: %s", err.what());
                sleep(3);
            }
        }
    }
}

void CRCommanderRos2::init()
{
    try
    {
        is_running_ = true;
        thread_ = std::unique_ptr<std::thread>(new std::thread(&CRCommanderRos2::recvTask, this));
    }
    catch (const TcpClientException &err)
    {
        RCLCPP_ERROR(kLogger, "Commander init failed: %s", err.what());
    }
}
int stringToInt(const std::string& str) {
    return std::atoi(str.c_str());
}
void CRCommanderRos2::doTcpCmd(std::shared_ptr<TcpClient> &tcp, const char *cmd, int32_t &err_id,
                               std::vector<std::string> &result)
{
    std::ignore = result;
    try
    {
        uint32_t has_read;
        char buf[kRecvBufSize];
        memset(buf, 0, sizeof(buf));
        auto currentTime = std::chrono::system_clock::now();
        auto currentTime_ms = std::chrono::time_point_cast<std::chrono::milliseconds>(currentTime);
        auto valueMS = currentTime_ms.time_since_epoch().count();
        RCLCPP_INFO(kLogger, "time: %ld  tcp send cmd: %s", valueMS, cmd);

        tcp->tcpSend(cmd, strlen(cmd));
        char *recv_ptr = buf;
        auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(2);
        while (true)
        {
            bool ok = tcp->tcpRecv(recv_ptr, static_cast<uint32_t>(kRecvBufSize - (recv_ptr - buf)), has_read, 100);
            if (!ok)
            {
                if (std::chrono::steady_clock::now() > deadline)
                {
                    RCLCPP_ERROR(kLogger, "doTcpCmd: response timeout for: %s", cmd);
                    err_id = -1;
                    return;
                }
                continue;
            }
            if (*(recv_ptr + has_read - 1) == ';')
                break;

            recv_ptr += has_read;
        }
        for (int i = 0; buf[i] != '\0'; i++)
        {
            if (buf[i] == '{')
            {
                std::string num_str(buf, i);
                while (!num_str.empty() && (num_str.back() == ',' || num_str.back() == ' '))
                    num_str.pop_back();
                err_id = stringToInt(num_str);
                RCLCPP_INFO(kLogger, "ErrorID: %s", num_str.c_str());
                break;
            }
        }

        RCLCPP_INFO(kLogger, "tcp recv feedback: %s", buf);
    }
    catch (const std::logic_error &err)
    {
        RCLCPP_ERROR(kLogger, "tcpDoCmd failed: %s", err.what());
    }
}


void CRCommanderRos2::doTcpCmd_f(std::shared_ptr<TcpClient> &tcp, const char *cmd, int32_t &err_id,std::string &mode_id,
                               std::vector<std::string> &result)
{
    std::ignore = result;
    try
    {
        uint32_t has_read;
        char buf[kRecvBufSize];
        memset(buf, 0, sizeof(buf));
        auto currentTime = std::chrono::system_clock::now();
        auto currentTime_ms = std::chrono::time_point_cast<std::chrono::milliseconds>(currentTime);
        auto valueMS = currentTime_ms.time_since_epoch().count();
        RCLCPP_INFO(kLogger, "time: %ld  tcp send cmd: %s", valueMS, cmd);
        tcp->tcpSend(cmd, strlen(cmd));
        char *recv_ptr = buf;
        auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(2);
        while (true)
        {
            bool ok = tcp->tcpRecv(recv_ptr, static_cast<uint32_t>(kRecvBufSize - (recv_ptr - buf)), has_read, 100);
            if (!ok)
            {
                if (std::chrono::steady_clock::now() > deadline)
                {
                    RCLCPP_ERROR(kLogger, "doTcpCmd_f: response timeout for: %s", cmd);
                    err_id = -1;
                    return;
                }
                continue;
            }
            if (*(recv_ptr + has_read - 1) == ';')
                break;

            recv_ptr += has_read;
        }
        int brace_start = 0;
        for (int i = 0; buf[i] != '\0'; i++)
        {
            if (buf[i] == '{')
            {
                std::string num_str(buf, i);
                while (!num_str.empty() && (num_str.back() == ',' || num_str.back() == ' '))
                    num_str.pop_back();
                err_id = stringToInt(num_str);
                RCLCPP_INFO(kLogger, "ErrorID: %d", err_id);
                brace_start = i;
            }
            if (buf[i] == '}')
            {
                mode_id = std::string(buf + brace_start, i - brace_start + 1);
                break;
            }
        }
        RCLCPP_INFO(kLogger, "tcp recv feedback: %s", buf);
    }
    catch (const std::logic_error &err)
    {
        RCLCPP_ERROR(kLogger, "tcpDoCmd failed: %s", err.what());
    }
}

bool CRCommanderRos2::callRosService(const std::string cmd, int32_t &err_id)
{
    std::lock_guard<std::mutex> lock(dash_mutex_);
    try
    {
        std::vector<std::string> result_;
        doTcpCmd(this->dash_board_tcp_, cmd.c_str(), err_id, result_);
        return true;
    }
    catch (const TcpClientException &err)
    {
        RCLCPP_ERROR(kLogger, "callRosService failed: %s", err.what());
        err_id = -1;
        return false;
    }
}
bool CRCommanderRos2::callRosService_f(const std::string cmd, int32_t &err_id,std::string &mode_id)
{
    std::lock_guard<std::mutex> lock(dash_mutex_);
    try
    {
        std::vector<std::string> result_;
        doTcpCmd_f(this->dash_board_tcp_, cmd.c_str(), err_id,mode_id, result_);
        return true;
    }
    catch (const TcpClientException &err)
    {
        RCLCPP_ERROR(kLogger, "callRosService_f failed: %s", err.what());
        err_id = -1;
        return false;
    }
}
bool CRCommanderRos2::callRosService(const std::string cmd, int32_t &err_id, std::vector<std::string> &result_)
{
    std::lock_guard<std::mutex> lock(dash_mutex_);
    try
    {
        doTcpCmd(this->dash_board_tcp_, cmd.c_str(), err_id, result_);
        return true;
    }
    catch (const TcpClientException &err)
    {
        RCLCPP_ERROR(kLogger, "callRosService failed: %s", err.what());
        err_id = -1;
        return false;
    }
}

bool CRCommanderRos2::sendServoCommand(const std::string &cmd, int32_t &err_id)
{
    std::lock_guard<std::mutex> lock(dash_mutex_);
    try
    {
        uint32_t has_read;
        char buf[kRecvBufSize];
        memset(buf, 0, sizeof(buf));

        dash_board_tcp_->tcpSend(cmd.c_str(), cmd.length());

        // Read response until ';' terminator (timeout after 2s).
        char *recv_ptr = buf;
        auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(2);
        while (true)
        {
            bool ok = dash_board_tcp_->tcpRecv(
                recv_ptr, static_cast<uint32_t>(kRecvBufSize - (recv_ptr - buf)),
                has_read, 100);
            if (!ok)
            {
                if (std::chrono::steady_clock::now() > deadline)
                {
                    RCLCPP_ERROR(kLogger, "sendServoCommand: response timeout");
                    err_id = -1;
                    return false;
                }
                continue;
            }
            if (*(recv_ptr + has_read - 1) == ';')
                break;
            recv_ptr += has_read;
        }

        // Parse error ID (number before first '{').
        err_id = 0;
        for (int i = 0; buf[i] != '\0'; i++)
        {
            if (buf[i] == '{')
            {
                std::string num_str(buf, i);
                // Trim trailing comma / whitespace
                while (!num_str.empty() &&
                       (num_str.back() == ',' || num_str.back() == ' '))
                    num_str.pop_back();
                err_id = std::atoi(num_str.c_str());
                break;
            }
        }

        return true;
    }
    catch (const std::logic_error &err)
    {
        RCLCPP_ERROR(kLogger, "sendServoCommand failed: %s", err.what());
        err_id = -1;
        return false;
    }
}

bool CRCommanderRos2::isEnable() const
{
    return real_time_data_->robot_mode == 5;
}

bool CRCommanderRos2::isConnected() const
{
    return dash_board_tcp_->isConnect() && real_time_tcp_->isConnect();
}

uint16_t CRCommanderRos2::getRobotMode() const
{
    return real_time_data_->robot_mode;
}

std::shared_ptr<RealTimeData> CRCommanderRos2::getRealData() const
{
    return real_time_data_;
}
