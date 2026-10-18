/**
  ******************************************************************************
  * @file    stm32f4xx_it.c
  * @brief   中断服务程序
  *
  * 说明：TIM6 与 CAN1_RX0 的中断会分别转发到 main.c 里的
  *       HAL_TIM_PeriodElapsedCallback / HAL_CAN_RxFifo0MsgPendingCallback，
  *       最终进入 ControlTask.cpp 的 MainTask() / CanFeedbackCallback()。
  ******************************************************************************
  */
#include "main.h"
#include "can.h"
#include "tim.h"

/* ---------------- Cortex-M4 内核异常 ---------------- */
void NMI_Handler(void)        { while (1) { } }
void HardFault_Handler(void)  { while (1) { } }   /* 不喂狗 -> IWDG 0.8s 后复位 */
void MemManage_Handler(void)  { while (1) { } }
void BusFault_Handler(void)   { while (1) { } }
void UsageFault_Handler(void) { while (1) { } }
void SVC_Handler(void)        { }
void DebugMon_Handler(void)   { }
void PendSV_Handler(void)     { }

void SysTick_Handler(void)
{
    HAL_IncTick();
}

/* ---------------- 外设中断 ---------------- */
void TIM6_DAC_IRQHandler(void)
{
    HAL_TIM_IRQHandler(&htim6);      /* -> HAL_TIM_PeriodElapsedCallback -> MainTask() */
}

void CAN1_RX0_IRQHandler(void)
{
    HAL_CAN_IRQHandler(&hcan1);      /* -> HAL_CAN_RxFifo0MsgPendingCallback -> CanFeedbackCallback() */
}
