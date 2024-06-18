#include <dobot_bringup/command.h>

CRCommanderRos2::CRCommanderRos2(const std::string &ip)
    : current_joint_{}, tool_vector_{}, is_running_(false)
{
    is_running_ = false;
    real_time_data_ = std::make_shared<RealTimeData>();
    real_time_tcp_ = std::make_shared<TcpClient>(ip, 30004);
    // dash_board_tcp_ = std::make_shared<TcpClient>(ip, 29999);
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
                    {
                        std::cout<<"[command] realtime data doesn't have expected size ,"<<real_time_data_->len<<std::endl;
                        continue;
                    }
                
                        

                    mutex_.lock();
                    for (uint32_t i = 0; i < 6; i++)
                        current_joint_[i] = deg2Rad(real_time_data_->q_actual[i]);
                    std::cout<<"[command] realtime data have expected size ,"<<real_time_data_->len<<std::endl;

                    memcpy(tool_vector_, real_time_data_->tool_vector_actual, sizeof(tool_vector_));
                    mutex_.unlock();
                }
                else
                {
                    std::cout << "tcp recv timeout" << std::endl;
                }
            }
            catch (const TcpClientException &err)
            {
                real_time_tcp_->disConnect();
                std::cout << "tcp recv error :" << std::endl;
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
                std::cout << "tcp recv Error : %s" << std::endl;
                sleep(3);
            }
        }

        // if (!dash_board_tcp_->isConnect())
        // {
        //     try
        //     {
        //         dash_board_tcp_->connect();
        //     }
        //     catch (const TcpClientException &err)
        //     {

        //         std::cout << "tcp recv ERROR : %s" << std::endl;
        //         sleep(3);
        //     }
        // }
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
        std::cout << "Commander : %s" << std::endl;
    }
}

void CRCommanderRos2::doTcpCmd(std::shared_ptr<TcpClient> &tcp, const char *cmd, int32_t &err_id,
                               std::vector<std::string> &result)
{
    try
    {
        uint32_t has_read;
        char buf[1024];
        memset(buf, 0, sizeof(buf));
        std::cout << "tcp send cmd :" << cmd << std::endl;
        tcp->tcpSend(cmd, strlen(cmd));

        // std::shared_ptr<std::string> recv_ptr = std::make_shared<std::string>();
        char *recv_ptr = buf;
        // while (true) {
        //     bool err = tcp->tcpRecv(recv_ptr, 1024, has_read, 0);
        //     if (!err) {
        //         sleep(0.01);
        //         continue;
        //     }
        //     if (*(recv_ptr + strlen(recv_ptr) - 1) == ';')
        //         break;

        //     recv_ptr = recv_ptr + strlen(recv_ptr);
        // }
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
        //result = regexRecv(std::string(*recv_ptr));
        if (result.size() >= 2U)
        {
            if (stoi(result[0]) == 0)
            {
                err_id = stoi(result[1]);
            }
            else
            {
                err_id = 2147483647; // int-max
            }
        }
        std::cout << "tcp recv feedback : " << *recv_ptr << std::endl; // FIXME parse the buf may be better
    }
    catch (const std::logic_error &err)
    {
        std::cout << "tcpDoCmd failed " << std::endl;
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
        std::cout << "%s" << std::endl;
        err_id = -1;
        return false;
    }
}
bool CRCommanderRos2::callRosService(const std::string cmd, int32_t &err_id, std::vector<std::string> &result_)
{
    try
    {
        // doTcpCmd(this->dash_board_tcp_, cmd.c_str(), err_id, result_);
        return true;
    }
    catch (const TcpClientException &err)
    {
        std::cout << "%s" << std::endl;
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
    return real_time_tcp_->isConnect();
}

uint16_t CRCommanderRos2::getRobotMode() const
{
    return real_time_data_->robot_mode;
}

std::shared_ptr<RealTimeData> CRCommanderRos2::getRealData() const
{
    return real_time_data_;
}
