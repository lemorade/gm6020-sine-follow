/**
  ******************************************************************************
  * @file    stm32f4xx_hal_msp.c
  * @brief   HAL 外设底层初始化（引脚复用、时钟、NVIC）
  *
  * 对应 C_project.ioc 里的引脚分配：
  *   PD0 = CAN1_RX (AF9), PD1 = CAN1_TX (AF9)
  *   PH0/PH1 = HSE 12MHz 晶振
  *   PD14   = 板载 LED
  ******************************************************************************
  */
#include "main.h"

/**
  * @brief CAN1 底层初始化
  */
void HAL_CAN_MspInit(CAN_HandleTypeDef *hcan)
{
    GPIO_InitTypeDef g = {0};

    if (hcan->Instance != CAN1) {
        return;
    }

    __HAL_RCC_CAN1_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();

    g.Mode      = GPIO_MODE_AF_PP;
    g.Pull      = GPIO_NOPULL;
    g.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
    g.Alternate = GPIO_AF9_CAN1;

    g.Pin = GPIO_PIN_0;                 /* PD0 = CAN1_RX */
    HAL_GPIO_Init(GPIOD, &g);

    g.Pin = GPIO_PIN_1;                 /* PD1 = CAN1_TX */
    HAL_GPIO_Init(GPIOD, &g);
}

void HAL_CAN_MspDeInit(CAN_HandleTypeDef *hcan)
{
    if (hcan->Instance != CAN1) {
        return;
    }
    HAL_GPIO_DeInit(GPIOD, GPIO_PIN_0 | GPIO_PIN_1);
    __HAL_RCC_CAN1_CLK_DISABLE();
}

/**
  * @brief TIM6 底层初始化（基础定时器，无引脚；只需开时钟与 NVIC）
  */
void HAL_TIM_Base_MspInit(TIM_HandleTypeDef *htim)
{
    if (htim->Instance != TIM6) {
        return;
    }

    __HAL_RCC_TIM6_CLK_ENABLE();

    /* 优先级分组已在 HAL_Init 里设为 4；控制节拍优先级给 2（高于一般任务） */
    HAL_NVIC_SetPriority(TIM6_DAC_IRQn, 2, 0);
    HAL_NVIC_EnableIRQ(TIM6_DAC_IRQn);
}

void HAL_TIM_Base_MspDeInit(TIM_HandleTypeDef *htim)
{
    if (htim->Instance != TIM6) {
        return;
    }
    __HAL_RCC_TIM6_CLK_DISABLE();
    HAL_NVIC_DisableIRQ(TIM6_DAC_IRQn);
}

/**
  * @brief IWDG 底层初始化：使能 LSI（独立看门狗时钟源）
  */
void HAL_IWDG_MspInit(IWDG_HandleTypeDef *hiwdg)
{
    (void)hiwdg;
    __HAL_RCC_LSI_ENABLE();
    while (__HAL_RCC_GET_FLAG(RCC_FLAG_LSIRDY) == RESET) {
        /* 等 LSI 稳定（约 40us） */
    }
}
