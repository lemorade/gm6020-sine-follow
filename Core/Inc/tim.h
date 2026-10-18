#ifndef __TIM_H
#define __TIM_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

/* TIM6：1 kHz 控制节拍
 *   APB1 定时器时钟 = 84 MHz
 *   Prescaler = 84 - 1 = 83 -> 计数频率 1 MHz
 *   Period    = 1000 - 1 = 999 -> 每 1 ms 溢出一次
 * 溢出中断里调用 MainTask()（MainTask 是 C++ 实现，见 ControlTask.cpp）
 */
extern TIM_HandleTypeDef htim6;

void MX_TIM6_Init(void);

#ifdef __cplusplus
}
#endif

#endif /* __TIM_H */
