/**
  ******************************************************************************
  * @file    stm32f4xx_hal_conf.h
  * @brief   HAL 配置头文件（最小化定制版）
  *
  * 说明：本工程只用到 RCC / GPIO / CAN / IWDG / FLASH / PWR / CORTEX / DMA
  *       这几个 HAL 模块，因此把 CubeMX 默认生成的模板裁剪到最小，
  *       只需要编译 Drivers/STM32F4xx_HAL_Driver/Src 下的 10 个 .c 文件。
  *
  *       如果你用 CubeMX 重新生成了工程，CubeMX 会覆盖本文件；
  *       记得把 HSE_VALUE 改回 12000000（乐迪/大疆开发板的晶振是 12 MHz），
  *       否则 CAN 波特率和系统时钟都会算错。
  ******************************************************************************
  */

#ifndef STM32F4xx_HAL_CONF_H
#define STM32F4xx_HAL_CONF_H

#ifdef __cplusplus
extern "C" {
#endif

/* ########################## Module Selection ############################## */
#define HAL_MODULE_ENABLED
#define HAL_CAN_MODULE_ENABLED
#define HAL_CORTEX_MODULE_ENABLED
#define HAL_DMA_MODULE_ENABLED
#define HAL_FLASH_MODULE_ENABLED
#define HAL_GPIO_MODULE_ENABLED
#define HAL_IWDG_MODULE_ENABLED
#define HAL_PWR_MODULE_ENABLED
#define HAL_RCC_MODULE_ENABLED
#define HAL_TIM_MODULE_ENABLED

/* ########################## Oscillator Values ############################# */
/**
  * @brief 外部高速晶振。大疆 RoboMaster 开发板（A 型/C 型）与乐迪开发板
  *        均为 12 MHz；STM32F4-Discovery 是 8 MHz。板子不同就改这里。
  */
#if !defined(HSE_VALUE)
  #define HSE_VALUE              12000000U
#endif

#if !defined(HSE_STARTUP_TIMEOUT)
  #define HSE_STARTUP_TIMEOUT    100U      /* ms */
#endif

#if !defined(HSI_VALUE)
  #define HSI_VALUE              16000000U
#endif

#if !defined(LSI_VALUE)
  #define LSI_VALUE              32000U    /* IWDG 时钟，看门狗溢出时间靠它 */
#endif

#if !defined(LSE_VALUE)
  #define LSE_VALUE              32768U
#endif

#if !defined(LSE_STARTUP_TIMEOUT)
  #define LSE_STARTUP_TIMEOUT    5000U
#endif

#if !defined(EXTERNAL_CLOCK_VALUE)
  #define EXTERNAL_CLOCK_VALUE   12288000U
#endif

/* ########################### System Configuration ######################### */
#define VDD_VALUE                     3300U
#define TICK_INT_PRIORITY             0x0FU
#define USE_RTOS                      0U
#define PREFETCH_ENABLE               1U
#define INSTRUCTION_CACHE_ENABLE      1U
#define DATA_CACHE_ENABLE             1U

#define USE_HAL_CAN_REGISTER_CALLBACKS     0U
#define USE_HAL_DMA_REGISTER_CALLBACKS     0U
#define USE_HAL_GPIO_REGISTER_CALLBACKS    0U
#define USE_HAL_IWDG_REGISTER_CALLBACKS    0U
#define USE_HAL_PWR_REGISTER_CALLBACKS     0U
#define USE_HAL_RCC_REGISTER_CALLBACKS     0U

/* #define USE_FULL_ASSERT    1U */

/* ########################## Assert Selection ############################## */
#ifdef USE_FULL_ASSERT
/**
  * @brief  The assert_param macro is used for function's parameters check.
  */
  #define assert_param(expr) ((expr) ? (void)0U : assert_failed((uint8_t *)__FILE__, __LINE__))
  void assert_failed(uint8_t *file, uint32_t line);
#else
  #define assert_param(expr) ((void)0U)
#endif /* USE_FULL_ASSERT */

/* ########################## Includes ###################################### */
#ifdef HAL_RCC_MODULE_ENABLED
  #include "stm32f4xx_hal_rcc.h"
#endif

#ifdef HAL_GPIO_MODULE_ENABLED
  #include "stm32f4xx_hal_gpio.h"
#endif

#ifdef HAL_DMA_MODULE_ENABLED
  #include "stm32f4xx_hal_dma.h"
#endif

#ifdef HAL_CORTEX_MODULE_ENABLED
  #include "stm32f4xx_hal_cortex.h"
#endif

#ifdef HAL_CAN_MODULE_ENABLED
  #include "stm32f4xx_hal_can.h"
#endif

#ifdef HAL_FLASH_MODULE_ENABLED
  #include "stm32f4xx_hal_flash.h"
  #include "stm32f4xx_hal_flash_ex.h"
  #include "stm32f4xx_hal_flash_ramfunc.h"
#endif

#ifdef HAL_TIM_MODULE_ENABLED
  #include "stm32f4xx_hal_tim.h"
#endif

#ifdef HAL_IWDG_MODULE_ENABLED
  #include "stm32f4xx_hal_iwdg.h"
#endif

#ifdef HAL_PWR_MODULE_ENABLED
  #include "stm32f4xx_hal_pwr.h"
  #include "stm32f4xx_hal_pwr_ex.h"
#endif

/* ########################## 断言与遗留宏 ################################## */
#include "stm32_hal_legacy.h"

#ifdef __cplusplus
}
#endif

#endif /* STM32F4xx_HAL_CONF_H */
