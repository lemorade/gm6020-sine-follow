#ifndef __GPIO_H
#define __GPIO_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

/* GPIO 初始化：C 板板载 LED 在 PD14（低电平点亮） */
void MX_GPIO_Init(void);

#ifdef __cplusplus
}
#endif

#endif /* __GPIO_H */
