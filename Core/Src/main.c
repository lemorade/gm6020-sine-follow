/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    main.c
  * @brief   GM6020 正弦跟随（C 板 STM32F407IGHx）
  *
  * 外设配置与 C_project.ioc 完全一致：
  *   HSE 12MHz -> PLL M=6 N=168 P=2 -> SYSCLK 168MHz, APB1 42MHz, APB2 84MHz
  *   CAN1 : PD0/PD1, Prescaler=3, BS1=6TQ, BS2=7TQ -> 1 Mbps
  *   TIM6 : Prescaler=84-1, Period=1000-1 -> 1 kHz 中断
  *   IWDG : Prescaler=64, Reload=400 -> 0.8 s
  *
  * 控制逻辑全在 C_board_src/ControlTask.cpp 里，本文件只负责初始化与转发回调。
  ******************************************************************************
  */
/* USER CODE END Header */

#include "main.h"
#include "can.h"
#include "tim.h"
#include "iwdg.h"
#include "gpio.h"

/* ============================================================================
 * C++ 实现的接口（ControlTask.h 用了 <cstdint>，C 文件不能直接 include，
 * 所以这里手工声明；实现者已用 extern "C" 导出，链接名一致）
 * ========================================================================== */
extern void ControlTaskInit(void);                                        /* 上电初始化一次 */
extern void MainTask(void);                                               /* 1kHz 控制入口   */
extern void CanFeedbackCallback(uint32_t std_id, const uint8_t *data);    /* CAN 反馈回调    */

/* 外设句柄的“实体”定义分别在 can.c / tim.c / iwdg.c 里，
 * 这里只通过 main.h 的 extern 声明引用，不要重复定义。 */

static void SystemClock_Config(void);

int main(void)
{
    /* 1) HAL 与系统时钟 */
    HAL_Init();
    SystemClock_Config();

    /* 2) 外设初始化（顺序不要乱：CAN/TIM 必须在 ControlTaskInit 之前） */
    MX_GPIO_Init();
    MX_CAN1_Init();      /* 只做 HAL_CAN_Init，过滤器与启动在 ControlTaskInit 里 */
    MX_TIM6_Init();      /* 配置 1kHz，但此处还不启动 */
    MX_IWDG_Init();      /* 启动看门狗：从此 0.8s 内必须有喂狗 */

    /* 3) 控制任务初始化：配置 CAN 过滤器、启动 CAN、启动 TIM6 中断。
     *    此后由 MainTask() 周期喂狗；CAN 无反馈导致喂狗停止时，
     *    系统在 IWDG 超时（约 0.8 s）后复位。 */
    ControlTaskInit();

    /* 4) 主循环：所有工作都在 TIM6 中断里完成，这里保持空转 */
    while (1) {
        /* 预留低优先级任务（串口打印、LED 心跳等） */
    }
}

/**
  * @brief 系统时钟：HSE 12MHz -> 168MHz
  */
static void SystemClock_Config(void)
{
    RCC_OscInitTypeDef osc = {0};
    RCC_ClkInitTypeDef clk = {0};

    __HAL_RCC_PWR_CLK_ENABLE();
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

    /* HSE 12MHz / PLLM 6 = 2MHz；2MHz * PLLN 168 = 336MHz；/PLLP 2 = 168MHz */
    osc.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    osc.HSEState       = RCC_HSE_ON;
    osc.PLL.PLLState   = RCC_PLL_ON;
    osc.PLL.PLLSource  = RCC_PLLSOURCE_HSE;
    osc.PLL.PLLM       = 6;
    osc.PLL.PLLN       = 168;
    osc.PLL.PLLP       = RCC_PLLP_DIV2;
    osc.PLL.PLLQ       = 7;
    if (HAL_RCC_OscConfig(&osc) != HAL_OK) {
        Error_Handler();
    }

    clk.ClockType      = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK |
                         RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    clk.SYSCLKSource   = RCC_SYSCLKSOURCE_PLLCLK;
    clk.AHBCLKDivider  = RCC_SYSCLK_DIV1;    /* 168 MHz */
    clk.APB1CLKDivider = RCC_HCLK_DIV4;      /* 42 MHz  */
    clk.APB2CLKDivider = RCC_HCLK_DIV2;      /* 84 MHz  */
    if (HAL_RCC_ClockConfig(&clk, FLASH_LATENCY_5) != HAL_OK) {
        Error_Handler();
    }
}

/**
  * @brief TIM6 周期中断回调（1 kHz）—— 1 kHz 控制入口
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM6) {
        MainTask();
    }
}

/**
  * @brief CAN1 FIFO0 收到报文回调 —— 把反馈转发给控制任务
  */
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
    CAN_RxHeaderTypeDef rx;
    uint8_t             data[8];

    if (hcan->Instance != CAN1) {
        return;
    }
    if (HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &rx, data) != HAL_OK) {
        return;
    }
    /* 按标准帧判断（0x205 落在标准帧范围内） */
    if (rx.IDE == CAN_ID_STD) {
        CanFeedbackCallback(rx.StdId, data);
    }
}

/**
  * @brief 错误处理：死循环，等 IWDG 复位
  */
void Error_Handler(void)
{
    __disable_irq();
    while (1) {
    }
}

#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line)
{
    (void)file;
    (void)line;
    Error_Handler();
}
#endif
