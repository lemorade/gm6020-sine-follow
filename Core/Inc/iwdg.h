#ifndef __IWDG_H
#define __IWDG_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

/* 独立看门狗句柄
 *   参数与 C_project.ioc 一致：Prescaler = 64, Reload = 400
 *   Tout = 64 * 400 / 32000Hz = 0.8 s
 * 一旦 MX_IWDG_Init() 成功，看门狗就无法用软件关闭：
 * 程序必须在 0.8s 内调用 HAL_IWDG_Refresh(&hiwdg)，否则系统复位。
 */
extern IWDG_HandleTypeDef hiwdg;

/* 启动 IWDG（在 main 里 MX_IWDG_Init 调用一次） */
void MX_IWDG_Init(void);

#ifdef __cplusplus
}
#endif

#endif /* __IWDG_H */
