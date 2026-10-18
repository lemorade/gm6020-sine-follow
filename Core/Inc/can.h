#ifndef __CAN_H
#define __CAN_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

/* CAN1 句柄（C 板：PD0 = CAN1_RX, PD1 = CAN1_TX, 1 Mbps） */
extern CAN_HandleTypeDef hcan1;

/* 初始化 CAN1：参数与 C_project.ioc 完全一致
 *   Prescaler = 3, BS1 = 6TQ, BS2 = 7TQ  -> 42MHz/3/(1+6+7) = 1 Mbps
 *   NART = ENABLE（自动重传）, TXFP = ENABLE（发送FIFO优先级）
 * 注意：过滤器与启动在 ControlTaskInit() 里完成（只收 0x205）
 */
void MX_CAN1_Init(void);

#ifdef __cplusplus
}
#endif

#endif /* __CAN_H */
