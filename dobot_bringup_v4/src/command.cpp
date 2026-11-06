#include <dobot_bringup/command.h>
#include <iostream>
#include <chrono>
#include <thread>
#include <rclcpp/rclcpp.hpp>
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
                    RCLCPP_WARN(rclcpp::get_logger("CRCommanderRos2"), "tcp recv timeout");
                }
            }
            catch (const TcpClientException &err)
            {
                real_time_tcp_->disConnect();
                RCLCPP_ERROR(rclcpp::get_logger("CRCommanderRos2"), "tcp recv error: %s", err.what());
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
                RCLCPP_ERROR(rclcpp::get_logger("CRCommanderRos2"), "tcp recv Error: %s", err.what());
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
                RCLCPP_ERROR(rclcpp::get_logger("CRCommanderRos2"), "tcp recv ERROR: %s", err.what());
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
        RCLCPP_ERROR(rclcpp::get_logger("CRCommanderRos2"), "Commander: %s", err.what());
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
        char buf[1024];
        memset(buf, 0, sizeof(buf));
        auto currentTime = std::chrono::system_clock::now();
        auto currentTime_ms = std::chrono::time_point_cast<std::chrono::milliseconds>(currentTime);
        auto valueMS = currentTime_ms.time_since_epoch().count();
        RCLCPP_INFO(rclcpp::get_logger("CRCommanderRos2"), "time: %ld  tcp send cmd : %s", valueMS, cmd);

        tcp->tcpSend(cmd, strlen(cmd));
        char *recv_ptr = buf;
        while (true)
        {
            bool err = tcp->tcpRecv(recv_ptr, 1024, has_read, 0);
            if (!err)
            {
                sleep(0.01);
                continue;
            }
            if (*(recv_ptr + strlen(recv_ptr) - 1) == ';')
                break;

            recv_ptr = recv_ptr + strlen(recv_ptr);
        }
        int data_len = strlen(buf);
        for (int i = 0; i < data_len; i++)
        {
            if (buf[i] == '{')
            {
                std::string str(buf);
                std::string result = str.substr(0, i-1);
                int num = stringToInt(result);
                err_id = num;
                RCLCPP_INFO(rclcpp::get_logger("CRCommanderRos2"), "ErrorID: %s", result.c_str());
                break;
            }
        }

        RCLCPP_INFO(rclcpp::get_logger("CRCommanderRos2"), "tcp recv feedback : %s", buf);
    }
    catch (const TcpClientException &err)
    {
        RCLCPP_ERROR(rclcpp::get_logger("CRCommanderRos2"), "tcpDoCmd failed: %s", err.what());
    }
}


void CRCommanderRos2::doTcpCmd_f(std::shared_ptr<TcpClient> &tcp, const char *cmd, int32_t &err_id,std::string &mode_id,
                               std::vector<std::string> &result)
{
    std::ignore = result;
    try
    {
        uint32_t has_read;
        char buf[1024];
        memset(buf, 0, sizeof(buf));
        auto currentTime = std::chrono::system_clock::now();
        auto currentTime_ms = std::chrono::time_point_cast<std::chrono::milliseconds>(currentTime);
        auto valueMS = currentTime_ms.time_since_epoch().count();
        RCLCPP_INFO(rclcpp::get_logger("CRCommanderRos2"), "time: %ld  tcp send cmd : %s", valueMS, cmd);
        tcp->tcpSend(cmd, strlen(cmd));
        char *recv_ptr = buf;
        while (true)
        {
            bool err = tcp->tcpRecv(recv_ptr, 1024, has_read, 0);
            if (!err)
            {
                sleep(0.01);
                continue;
            }
            if (*(recv_ptr + strlen(recv_ptr) - 1) == ';')
                break;

            recv_ptr = recv_ptr + strlen(recv_ptr);
        }
        int pose1 = 0;
        int data_len = strlen(buf);
        for (int i = 0; i < data_len; i++)
        {
            if (buf[i] == '{')
            {
                std::string str(buf);
                std::string result = str.substr(0, i-1);
                int num = stringToInt(result);
                err_id = num;
                RCLCPP_INFO(rclcpp::get_logger("CRCommanderRos2"), "ErrorID: %d", num);
                pose1 = i;
            }
            if (buf[i] == '}')
            {
                std::string str(buf);
                std::string result = str.substr(pose1, i-pose1+1);
                mode_id = result;
                break;
            }
        }
        RCLCPP_INFO(rclcpp::get_logger("CRCommanderRos2"), "tcp recv feedback : %s", buf);
    }
    catch (const TcpClientException &err)
    {
        RCLCPP_ERROR(rclcpp::get_logger("CRCommanderRos2"), "tcpDoCmd_f failed: %s", err.what());
    }
}

bool CRCommanderRos2::callRosService(const std::string cmd, int32_t &err_id)
{
    try
    {
        std::vector<std::string> result_;
        doTcpCmd(this->dash_board_tcp_, cmd.c_str(), err_id, result_);
        return true;
    }
    catch (const TcpClientException &err)
    {
        RCLCPP_ERROR(rclcpp::get_logger("CRCommanderRos2"), "callRosService: %s", err.what());
        err_id = -1;
        return false;
    }
}
bool CRCommanderRos2::callRosService_f(const std::string cmd, int32_t &err_id,std::string &mode_id)
{
    try
    {
        std::vector<std::string> result_;
        doTcpCmd_f(this->dash_board_tcp_, cmd.c_str(), err_id,mode_id, result_);
        return true;
    }
    catch (const TcpClientException &err)
    {
        RCLCPP_ERROR(rclcpp::get_logger("CRCommanderRos2"), "callRosService_f: %s", err.what());
        err_id = -1;
        return false;
    }
}
bool CRCommanderRos2::callRosService(const std::string cmd, int32_t &err_id, std::vector<std::string> &result_)
{
    try
    {
        doTcpCmd(this->dash_board_tcp_, cmd.c_str(), err_id, result_);
        return true;
    }
    catch (const TcpClientException &err)
    {
        RCLCPP_ERROR(rclcpp::get_logger("CRCommanderRos2"), "callRosService: %s", err.what());
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
    bool dash_connected = dash_board_tcp_->isConnect();
    bool real_time_connected = real_time_tcp_->isConnect();
    
    // 仅在连接断开时打印日志，帮助分析问题
    static bool last_connected = true;
    bool current_connected = dash_connected && real_time_connected;
    
    if (!current_connected && last_connected) {
        RCLCPP_WARN(rclcpp::get_logger("CRCommanderRos2"), 
                    "Robot disconnected - Dashboard: %s, Real-time: %s", 
                    dash_connected ? "connected" : "disconnected", 
                    real_time_connected ? "connected" : "disconnected");
    }
    last_connected = current_connected;
    
    return current_connected;
}

uint16_t CRCommanderRos2::getRobotMode() const
{
    return real_time_data_->robot_mode;
}

std::shared_ptr<RealTimeData> CRCommanderRos2::getRealData() const
{
    return real_time_data_;
}
