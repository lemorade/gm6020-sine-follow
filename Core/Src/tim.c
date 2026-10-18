/**
  ******************************************************************************
  * @file    tim.c
  * @brief   TIM6 初始化：1 kHz 控制节拍
  *
  * APB1 定时器时钟 = 84 MHz（APB1 42MHz，定时器时钟 x2）
  *   Prescaler = 84 - 1 = 83   -> 计数频率 1 MHz
  *   Period    = 1000 - 1 = 999 -> 1 ms 溢出一次，即 1 kHz
  *
  * 溢出中断里会调用 HAL_TIM_PeriodElapsedCallback -> MainTask()（见 main.c）
  ******************************************************************************
  */
#include "tim.h"

TIM_HandleTypeDef htim6;

void MX_TIM6_Init(void)
{
    TIM_ClockConfigTypeDef  sClockSourceConfig = {0};
    TIM_MasterConfigTypeDef sMasterConfig      = {0};

    htim6.Instance               = TIM6;
    htim6.Init.Prescaler         = 84 - 1;      /* 84MHz -> 1MHz */
    htim6.Init.CounterMode       = TIM_COUNTERMODE_UP;
    htim6.Init.Period            = 1000 - 1;    /* 1MHz / 1000 = 1kHz */
    htim6.Init.ClockDivision     = TIM_CLOCKDIVISION_DIV1;
    htim6.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    if (HAL_TIM_Base_Init(&htim6) != HAL_OK) {
        Error_Handler();
    }

    sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
    if (HAL_TIM_ConfigClockSource(&htim6, &sClockSourceConfig) != HAL_OK) {
        Error_Handler();
    }

    sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
    sMasterConfig.MasterSlaveMode     = TIM_MASTERSLAVEMODE_DISABLE;
    if (HAL_TIMEx_MasterConfigSynchronization(&htim6, &sMasterConfig) != HAL_OK) {
        Error_Handler();
    }

    /* 注意：这里只配置，不启动；启动在 ControlTaskInit() 里
     *       HAL_TIM_Base_Start_IT(&htim6); */
}
