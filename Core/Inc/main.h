#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f4xx_hal.h"

/* ----------------------------------------------------------------------------
 * 外设句柄（与 CubeMX 生成 main.h 的写法一致）
 *
 * ControlTask.cpp 中引用的是 "main.h"/"can.h"/"iwdg.h"，
 * 但用到了 htim6，所以 main.h 必须把三个句柄都声明出来，
 * 否则会报 "'htim6' was not declared in this scope"。
 * -------------------------------------------------------------------------- */
extern CAN_HandleTypeDef  hcan1;
extern TIM_HandleTypeDef  htim6;
extern IWDG_HandleTypeDef hiwdg;

/* 错误处理（HAL 回调里出错时进入；正式运行时会被 IWDG 复位） */
void Error_Handler(void);

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
