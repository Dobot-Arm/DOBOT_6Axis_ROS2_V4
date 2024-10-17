/**
 ***********************************************************************************************************************
 *
 * @author qiufengwu
 * @date   2024/01/23
 *
 *
 ***********************************************************************************************************************
 */

#ifndef COMMANDER_H
#define COMMANDER_H

#include <vector>
#include <string>
#include <memory>
#include <thread>
#include <mutex>
#include <algorithm>
#include <regex>
#include <assert.h>
#include <cstring>
#include <dobot_bringup/tcp_socket.h>

#pragma pack(push, 1)
// 数据 按照 8 字节 以及  48 字节对齐的模式,
// 大小设计为  30 * 8 * 6 = 30 *6*sizeof(double) = 30 * sizeof(double)
typedef struct RealTimeData_t
{
    //
    uint16_t len;                // 0000 ~ 0001  字符长度
    uint16_t Reserve[3];         // 0002 ~ 0007  占位符
    uint64_t digital_input_bits; // 0008 ~ 0015  DI 按照bit 进行计算的
    uint64_t digital_outputs;    // 0016 ~ 0023  DO 按照bit 进行计算的
    uint64_t robot_mode;         // 0024 ~ 0031  机器人模式
    uint64_t controller_timer;   // 0032 ~ 0039  机器人时间 1970年到现在的时间  单位是 ms
    uint64_t run_time;           // 0040 ~ 0047  机器人开机时间 单位是ms
    // 0048 ~ 0095                       //
    uint64_t test_value;         // 0048 ~ 0055  内存结构测试标准值  0x0123 4567 89AB CDEF
    double safety_mode;          // 0056 ~ 0063  (弃用字段)
    double speed_scaling;        // 0064 ~ 0071  全局速率
    double linear_momentum_norm; // 0072 ~ 0079  机器人当前动量(未实现)
    double v_main;               // 0080 ~ 0087  控制板电压值(未实现)
    double v_robot;              // 0088 ~ 0095  机器人电压(48V)
    // 0096 ~ 0143                       //
    double i_robot;                      // 0096 ~ 0103 机器人电流
    double program_state;                // 0104 ~ 0111 脚本运行状态
    double safety_status;                // 0112 ~ 0119 安全状态（未实现）
    double tool_accelerometer_values[3]; // 0120 ~ 0143 tcp加速度（未实现）
    // 0144 ~ 0191                       //
    double elbow_position[3]; // 0144 ~ 0167 肘位置（未实现）
    double elbow_velocity[3]; // 0168 ~ 0191 肘速度（未实现）
    // 0192 ~ ...                        //
    double q_target[6];           // 0192 ~ 0239  // 目标关节位置
    double qd_target[6];          // 0240 ~ 0287  // 目标关节速度
    double qdd_target[6];         // 0288 ~ 0335  // 目标关节加速度
    double i_target[6];           // 0336 ~ 0383  // 目标关节电流
    double m_target[6];           // 0384 ~ 0431  // 目标关节扭矩
    double q_actual[6];           // 0432 ~ 0479  // 实际关节位置
    double qd_actual[6];          // 0480 ~ 0527  // 实际关节速度
    double i_actual[6];           // 0528 ~ 0575  // 实际电流
    double i_control[6];          // 0576 ~ 0623  // TCP传感器力值（未实现）
    double tool_vector_actual[6]; // 0624 ~ 0671  // TCP实际坐标 (TCP: 末端工具中心点 terminal central point)
    double TCP_speed_actual[6];   // 0672 ~ 0719  // TCP速度
    double TCP_force[6];          // 0720 ~ 0767  // TCP力值  (电流环计算)
    double tool_vector_target[6]; // 0768 ~ 0815  // TCP目标坐标
    double TCP_speed_target[6];   // 0816 ~ 0863  // TCP目标速度
    double motor_temperatures[6]; // 0864 ~ 0911  // 关节温度
    double joint_modes[6];        // 0912 ~ 0959  // 关节控制模式
    double v_actual[6];           // 960  ~ 1007  // 关节电压
    int8_t handtype[4];           // 1008,1009,1010,1011 R、D、N、cfg   手系信息  新版已经删除????
    int8_t userCoordinate;        // 1012 用户坐标系ID
    int8_t toolCoordinate;        // 1013 工具坐标系ID
    int8_t isRunQueuedCmd;        // 1014 算法队列运行标志
    int8_t isPauseCmdFlag;        // 1015 算法队列暂停标志
    int8_t velocityRatio;         // 1016 关节速度比例
    int8_t accelerationRatio;     // 1017 关节加速度比例
    int8_t jerkRatio;             // 1018 关节加加速度比例（未实现）
    int8_t xyzVelocityRatio;      // 1019 笛卡尔位置速度比例 (x,y,z  单位是 距离每秒)
    int8_t rVelocityRatio;        // 1020 笛卡尔姿态速度比例 (rx,ry,rz 单位是 角度每秒)
    int8_t xyzAccelerationRatio;  // 1021 笛卡尔位置加速度比例
    int8_t rAccelerationRatio;    // 1022 笛卡尔姿态加速度比例
    int8_t xyzJerkRatio;          // 1023 笛卡尔位置加加速度比例（未实现）
    int8_t rJerkRatio;            // 1024 笛卡尔姿态加加速度比例（未实现）
    int8_t BrakeStatus;           // 1025 机器人抱闸状态
    int8_t EnableStatus;          // 1026 机器人使能状态
    int8_t DragStatus;            // 1027 机器人拖拽状态
    int8_t RunningStatus;         // 1028 机器人运行状态
    int8_t ErrorStatus;           // 1029 机器人报警状态
    int8_t JogStatus;             // 1030 机器人点动状态
    int8_t RobotType;             // 1031 M1机型手系
    int8_t DragButtonSignal;      // 1032 按钮板拖拽信号
    int8_t EnableButtonSignal;    // 1033 按钮板使能信号
    int8_t RecordButtonSignal;    // 1034 按钮板录制信号
    int8_t ReappearButtonSignal;  // 1035 按钮板复现信号
    int8_t JawButtonSignal;       // 1036 按钮板夹爪控制信号
    int8_t SixForceOnline;        // 1037 六维力在线状态（未实现）
    int8_t CollisionStates;       // 1038 碰撞状态
    int8_t ArmApproachState;      // 1039 小臂接近暂停状态
    int8_t J4ApproachState;       // 1040 J4接近暂停状态
    int8_t J5ApproachState;       // 1041 J5接近暂停状态
    int8_t J6ApproachState;       // 1042 J6接近暂停状态
    int8_t Reserve2[61];          // 1043 ~ 1103   预留
    double vibrationDisZ;         // 1104 ~ 1111 加速度计测量Z轴抖动位移
    uint64_t currentCommandId;    // 1112 ~ 1119 当前运动队列id
    double m_actual[6];           // 1120 ~ 1167 实际扭矩
    double load;                  // 1168 ~ 1175 负载质量
    double centerX;               // 1176 ~ 1183 负载 X 偏心距离
    double centerY;               // 1184 ~ 1191 负载 Y 偏心 距离
    double centerZ;               // 1192 ~ 1199 负载 Z 偏心距离
    double user[6];               // 1200 ~ 1247 用户坐标系值
    double tool[6];               // 1248 ~ 1295 工具坐标系值
    double TraceIndex;            // 1296 ~ 1303 轨迹复现索引 （未实现）
    double SixForceValue[6];      // 1304 ~ 1351 六维力传感器原始值
    double TargetQuaternion[4];   // 1352 ~ 1383 目标四元数
    double ActualQuaternion[4];   // 1384 ~ 1415 实际四元数
    uint16_t AutoManualMode;      // 1416 ~ 1417 手自动模式 0: 未开启 1: manual 2:auto
    int8_t Reserve3[22];          // 1418 ~ 1439
} RealTimeData;
#pragma pack(pop)

class CRCommanderRos2
{
protected:
    static constexpr double PI = 3.1415926;

private:
    std::mutex mutex_;
    double current_joint_[6];
    double tool_vector_[6];
    std::shared_ptr<RealTimeData> real_time_data_;
    std::atomic<bool> is_running_;
    std::unique_ptr<std::thread> thread_;
    std::shared_ptr<TcpClient> real_time_tcp_;
    std::shared_ptr<TcpClient> dash_board_tcp_;

public:
    explicit CRCommanderRos2(const std::string &ip);

    ~CRCommanderRos2();
    void getCurrentJointStatus(double *joint);
    void getToolVectorActual(double *val);
    void recvTask();
    void init();
    bool callRosService(const std::string cmd, int32_t &err_id);
    bool callRosService_f(const std::string cmd, int32_t &err_id,std::string &mode_id);
    bool callRosService(const std::string cmd, int32_t &err_id, std::vector<std::string> &result_);
    bool isEnable() const;
    bool isConnected() const;
    uint16_t getRobotMode() const;
    std::shared_ptr<RealTimeData> getRealData() const;

private:
    static void doTcpCmd(std::shared_ptr<TcpClient> &tcp, const char *cmd, int32_t &err_id,
                         std::vector<std::string> &result);
    static void doTcpCmd_f(std::shared_ptr<TcpClient> &tcp, const char *cmd, int32_t &err_id,std::string &mode_id,
                         std::vector<std::string> &result);
    static inline double rad2Deg(double rad)
    {
        return rad * 180.0 / PI;
    }

    static inline double deg2Rad(double deg)
    {
        return deg * PI / 180.0;
    }
};

#endif // COMMANDER_H
