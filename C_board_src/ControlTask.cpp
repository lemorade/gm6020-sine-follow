#include "ControlTask.h"
#include "Gm6020.h"
#include "Pid.h"

#include "main.h"           // CubeMX 生成: hcan1, hiwdg 等句柄
#include "can.h"
#include "iwdg.h"

#include <cmath>

// ============================================================================
// 可调参数区 (组长要改曲线/增益只需动这里)
// ============================================================================

namespace {

// ---- 正弦曲线 ----
volatile float g_sine_freq = 0.5f;   // 频率 Hz (速度环/位置环共用)
volatile float g_vel_amp   = 2.0f;   // 速度正弦幅值 rad/s
volatile float g_pos_amp   = 1.0f;   // 位置正弦幅值 rad

constexpr float kPi      = 3.14159265358979323846f;
constexpr float kTwoPi   = 2.0f * kPi;

// ---- 控制周期 ----
constexpr float kCtrlDt  = 0.001f;   // 1kHz
constexpr float kAmpLimit = 25000.0f;  // GM6020 电压给定上限
constexpr float kMinAmp   = -25000.0f;

// ---- CAN ----
constexpr uint8_t kMotorId = 1;      // GM6020 拨码 ID
constexpr uint32_t kTxStdId = 0x1FF; // ID 1~4 的电压控制帧标识符

// ---- 通信超时(看门狗保护) ----
// 若超过该时间未收到电机反馈(掉线/总线故障), 视为异常:
//   1) 输出电压立即归零
//   2) 停止喂 IWDG -> 约 0.8s 后自动复位 (IWDG 超时见 .ioc: Prescaler=64, Reload=400)
constexpr uint32_t kFeedbackTimeoutMs = 100;

} // namespace

// ---- 全局对象 ----
static Gm6020  g_motor(kMotorId);

// 速度环: 输入速度误差(rad/s), 输出电机电压给定
static Pid     g_velPid(  1200.0f, 30.0f,  0.0f, kAmpLimit, kMinAmp);
// 位置环: 输入角度误差(rad),   输出目标速度(rad/s), 再由速度环执行
static Pid     g_posPid(     8.0f,  0.0f,  0.35f,  6.0f,     -6.0f);

static CtrlMode g_mode      = CTRL_MODE_VEL;
static uint8_t  g_enabled   = 1;

static float    g_timeS     = 0.0f;   // 控制器运行时间(s), 用于生成正弦
static uint32_t g_lastFbMs  = 0;      // 最近一次收到反馈的时间戳
static uint8_t  g_hasFb     = 0;      // 是否收到过反馈

// ============================================================================
// 内部工具
// ============================================================================

// 取当前毫秒时间戳 (基于 HAL_GetTick)
static inline uint32_t nowMs(void) {
    return HAL_GetTick();
}

// 发送本电机的电压控制帧
static void sendVoltage(int16_t voltage) {
    uint8_t data[8] = {0};
    g_motor.setVoltage(voltage);
    g_motor.encode(data);

    CAN_TxHeaderTypeDef tx{};
    tx.StdId = kTxStdId;
    tx.IDE   = CAN_ID_STD;
    tx.RTR   = CAN_RTR_DATA;
    tx.DLC   = 8;

    uint32_t mailbox = 0;
    // 邮箱满时直接丢弃本拍(1kHz 下偶尔丢帧无碍), 不阻塞
    (void)HAL_CAN_AddTxMessage(&hcan1, &tx, data, &mailbox);
}

// ============================================================================
// 对外接口
// ============================================================================

extern "C" void ControlTaskInit(void) {
    // CAN1 过滤器: 只接收本电机反馈帧 (0x204 + ID = 0x205)
    CAN_FilterTypeDef filter{};
    filter.FilterMode            = CAN_FILTERMODE_IDMASK;
    filter.FilterScale           = CAN_FILTERSCALE_32BIT;
    filter.FilterIdHigh          = (g_motor.rxId() << 5);
    filter.FilterIdLow           = 0;
    filter.FilterMaskIdHigh      = (0x7FF << 5);  // 全比较
    filter.FilterMaskIdLow       = 0;
    filter.FilterFIFOAssignment  = CAN_RX_FIFO0;
    filter.FilterBank            = 0;
    filter.FilterActivation      = ENABLE;
    filter.SlaveStartFilterBank  = 14;   // F4 双 CAN 时由 CAN2 使用, 保留默认
    if (HAL_CAN_ConfigFilter(&hcan1, &filter) != HAL_OK) {
        Error_Handler();
    }

    // 启动 CAN1 + 开启 FIFO0 接收中断
    if (HAL_CAN_Start(&hcan1) != HAL_OK) {
        Error_Handler();
    }
    if (HAL_CAN_ActivateNotification(&hcan1, CAN_IT_RX_FIFO0_MSG_PENDING) != HAL_OK) {
        Error_Handler();
    }

    // 启动 1kHz 控制定时器 (TIM6)
    HAL_TIM_Base_Start_IT(&htim6);

    // 复位状态
    g_timeS    = 0.0f;
    g_hasFb    = 0;
    g_lastFbMs = nowMs();
    g_velPid.reset();
    g_posPid.reset();
}

extern "C" void CanFeedbackCallback(uint32_t std_id, const uint8_t *data) {
    if (std_id != g_motor.rxId()) {
        return;   // 非本电机
    }
    g_motor.decode(data);
    g_hasFb    = 1;
    g_lastFbMs = nowMs();
}

extern "C" void MainTask(void) {
    // ------------------------------------------------------------------
    // 1. 看门狗保护: 判断反馈是否超时
    // ------------------------------------------------------------------
    uint32_t now = nowMs();
    uint8_t comm_ok = (g_hasFb != 0) &&
                      ((now - g_lastFbMs) <= kFeedbackTimeoutMs);

    if (!comm_ok) {
        // 通信异常: 立即卸载输出, 并"停止喂狗"
        // 不再调用 HAL_IWDG_Refresh, IWDG 将在约 0.8s 后复位系统
        sendVoltage(0);
        g_velPid.reset();
        g_posPid.reset();
        return;
    }

    // 通信正常 -> 喂一次独立看门狗(参考博客: 正常运行期间定时 Refresh)
    HAL_IWDG_Refresh(&hiwdg);

    // ------------------------------------------------------------------
    // 2. 失能时直接输出 0
    // ------------------------------------------------------------------
    if (!g_enabled) {
        sendVoltage(0);
        return;
    }

    // ------------------------------------------------------------------
    // 3. 生成正弦目标并执行对应控制环
    // ------------------------------------------------------------------
    g_timeS += kCtrlDt;
    float sine = sinf(kTwoPi * g_sine_freq * g_timeS);

    float voltage = 0.0f;

    if (g_mode == CTRL_MODE_VEL) {
        // ---- 速度环跟随 ----
        // 目标速度 v_ref = A*sin(wt), 反馈速度 v_fdb (rad/s)
        float v_ref = g_vel_amp * sine;
        float v_fdb = g_motor.vel();
        voltage = g_velPid.calc(v_ref, v_fdb);
    } else {
        // ---- 位置环跟随 ----
        // 目标位置 th_ref = A*sin(wt), 反馈位置 th_fdb (rad, 累积)
        float th_ref = g_pos_amp * sine;
        float th_fdb = g_motor.angle();

        // 位置环输出"目标速度", 再交给速度环作为内环执行 (串级)
        float v_ref = g_posPid.calc(th_ref, th_fdb);
        float v_fdb = g_motor.vel();
        voltage = g_velPid.calc(v_ref, v_fdb);
    }

    // 电压限幅后下发
    if (voltage > kAmpLimit) voltage = kAmpLimit;
    if (voltage < kMinAmp)   voltage = kMinAmp;
    sendVoltage((int16_t)voltage);
}

extern "C" void CtrlSetMode(enum CtrlMode mode) {
    g_mode = mode;
    g_timeS = 0.0f;      // 切换模式时曲线相位归零
    g_velPid.reset();
    g_posPid.reset();
}

extern "C" enum CtrlMode CtrlGetMode(void) {
    return g_mode;
}

extern "C" void CtrlSetSine(float freq_hz, float amplitude) {
    if (freq_hz > 0.0f) {
        g_sine_freq = freq_hz;
    }
    if (g_mode == CTRL_MODE_VEL) {
        g_vel_amp = amplitude;
    } else {
        g_pos_amp = amplitude;
    }
}

extern "C" void CtrlEnable(uint8_t enable) {
    g_enabled = enable ? 1 : 0;
    if (!g_enabled) {
        g_velPid.reset();
        g_posPid.reset();
    }
}
