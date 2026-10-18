/**
  ******************************************************************************
  * @file    iwdg.c
  * @brief   独立看门狗初始化
  *
  * 参数与 C_project.ioc 一致：
  *   Prescaler = 64, Reload = 400
  *   Tout = 64 * 400 / LSI(32000Hz) = 0.8 s
  *
  * 喂狗位置：ControlTask.cpp 的 MainTask() 中，通信正常时每 1ms 调
  *   HAL_IWDG_Refresh(&hiwdg);
  * 一旦 CAN 反馈超时（>100ms）或程序卡死，停止喂狗 -> 0.8s 后系统复位。
  ******************************************************************************
  */
#include "iwdg.h"

IWDG_HandleTypeDef hiwdg;

void MX_IWDG_Init(void)
{
    hiwdg.Instance       = IWDG;
    hiwdg.Init.Prescaler = IWDG_PRESCALER_64;   /* LSI/64 = 500 Hz */
    hiwdg.Init.Reload    = 400;                 /* 400 / 500Hz = 0.8 s */

    if (HAL_IWDG_Init(&hiwdg) != HAL_OK) {
        Error_Handler();
    }
}
