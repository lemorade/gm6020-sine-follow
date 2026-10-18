/*
 * @Author: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @Date: 2026-09-27 19:11:11
 * @LastEditors: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @LastEditTime: 2026-09-27 22:07:01
 * @FilePath: \第一部分考核\C_board_src\ControlTask.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#ifndef CONTROLTASK_H
#define CONTROLTASK_H

#include <cstdint>

// ============================================================================
// GM6020 正弦跟随控制任务
//   - 速度环: 让电机转速跟随正弦速度曲线  v_ref(t) = A_v * sin(2*pi*f*t)
//   - 位置环: 让电机角度跟随正弦位置曲线  th_ref(t) = A_p * sin(2*pi*f*t)
//   两条曲线均以 0 为中心摆动，频率/幅值可在 ControlTask.cpp 顶部常数区修改。
//
//   硬件假设: GM6020 电机 ID = 1，接在 CAN1 (PD0/PD1)
//   控制周期: 1kHz (TIM6)，由 MainTask() 驱动
// ============================================================================

// 控制模式选择
enum CtrlMode {
    CTRL_MODE_VEL = 0,   // 速度环: 跟随正弦速度曲线
    CTRL_MODE_POS = 1    // 位置环: 跟随正弦位置曲线
};

// 供 C 中断处理(stm32f4xx_it.c 及 HAL 弱回调)调用的接口
#ifdef __cplusplus
extern "C" {
#endif

// 上电初始化: 配置 CAN 过滤器、启动 CAN 与 1kHz 控制定时器
// 在 main.c 中 MX_XXX_Init() 全部执行完后调用一次
void ControlTaskInit(void);

// 1kHz 控制入口, 在 TIM6 周期中断(HAL_TIM_PeriodElapsedCallback)中调用
void MainTask(void);

// CAN 接收回调, 在 HAL_CAN_RxFifo0MsgPendingCallback 中调用
void CanFeedbackCallback(uint32_t std_id, const uint8_t *data);

// ---- 运行期控制接口 (可选, 供上位机/按键/调试器调用) ----

// 切换控制模式
void CtrlSetMode(enum CtrlMode mode);

// 获取当前控制模式
enum CtrlMode CtrlGetMode(void);

// 设置正弦曲线的频率(Hz)与幅值
//   速度模式: 幅值单位 rad/s
//   位置模式: 幅值单位 rad
void CtrlSetSine(float freq_hz, float amplitude);

// 使能/失能输出 (失能时电压输出置 0)
void CtrlEnable(uint8_t enable);

#ifdef __cplusplus
}
#endif

#endif // CONTROLTASK_H
