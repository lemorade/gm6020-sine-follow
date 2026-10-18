/**
  ******************************************************************************
  * @file    can.c
  * @brief   CAN1 初始化（C 板：PD0 = CAN1_RX, PD1 = CAN1_TX）
  *
  * 参数与 C_project.ioc 一致：
  *   Prescaler = 3, BS1 = 6TQ, BS2 = 7TQ
  *   -> 42MHz / 3 / (1 + 6 + 7) = 1 Mbps
  *   NART = ENABLE(AutoRetransmission), TXFP = ENABLE(TransmitFifoPriority)
  *
  * 过滤器与 HAL_CAN_Start 在 ControlTask.cpp 的 ControlTaskInit() 里完成
  * （它需要知道电机 ID 才能只收 0x205）。
  ******************************************************************************
  */
#include "can.h"

CAN_HandleTypeDef hcan1;

void MX_CAN1_Init(void)
{
    hcan1.Instance                  = CAN1;
    hcan1.Init.Prescaler            = 3;                 /* APB1 42MHz / 3 = 14MHz */
    hcan1.Init.Mode                 = CAN_MODE_NORMAL;
    hcan1.Init.SyncJumpWidth        = CAN_SJW_1TQ;
    hcan1.Init.TimeSeg1             = CAN_BS1_6TQ;
    hcan1.Init.TimeSeg2             = CAN_BS2_7TQ;
    hcan1.Init.TimeTriggeredMode    = DISABLE;
    hcan1.Init.AutoBusOff           = DISABLE;
    hcan1.Init.AutoWakeUp           = DISABLE;
    hcan1.Init.AutoRetransmission   = ENABLE;            /* ioc: NART = ENABLE */
    hcan1.Init.ReceiveFifoLocked    = DISABLE;
    hcan1.Init.TransmitFifoPriority = ENABLE;            /* ioc: TXFP = ENABLE */

    if (HAL_CAN_Init(&hcan1) != HAL_OK) {
        Error_Handler();
    }
}
