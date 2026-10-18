/**
  ******************************************************************************
  * @file    gpio.c
  * @brief   GPIO 初始化：C 板板载 LED（PD14，低电平点亮）
  ******************************************************************************
  */
#include "gpio.h"

void MX_GPIO_Init(void)
{
    GPIO_InitTypeDef g = {0};

    /* 打开所有可能用到的端口时钟（CAN 用 D 口，LED 用 D 口，HSE 用 H 口） */
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    __HAL_RCC_GPIOE_CLK_ENABLE();
    __HAL_RCC_GPIOF_CLK_ENABLE();
    __HAL_RCC_GPIOG_CLK_ENABLE();
    __HAL_RCC_GPIOH_CLK_ENABLE();

    /* 板载 LED：PD14，推挽输出，默认熄灭（高电平） */
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_SET);
    g.Pin   = GPIO_PIN_14;
    g.Mode  = GPIO_MODE_OUTPUT_PP;
    g.Pull  = GPIO_NOPULL;
    g.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOD, &g);
}
