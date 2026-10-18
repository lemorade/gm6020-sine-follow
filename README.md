# GM6020 正弦速度/位置跟随控制

基于 STM32F407 + HAL 库的 RoboMaster GM6020 直流无刷电机控制程序。
实现电机**正弦速度曲线跟随**与**正弦位置曲线跟随**，并加入**独立看门狗(IWDG)保护**。

## 功能

- **速度环**：电机转速跟随正弦速度曲线 `v_ref(t) = A_v · sin(2π·f·t)`（rad/s）
- **位置环**：电机角度跟随正弦位置曲线 `θ_ref(t) = A_p · sin(2π·f·t)`（rad）
- 两条曲线均**以 0 为中心摆动**，频率/幅值可配置
- 通过枚举变量 `CtrlMode` 在两种模式间切换
- **看门狗保护**：
  - 程序正常运行期间，1kHz 任务定时喂狗 `HAL_IWDG_Refresh(&hiwdg)`
  - CAN 反馈超时（>100ms，掉线/总线故障）→ 输出立即归零 + 停止喂狗 → 约 0.8s 后 IWDG 复位系统

## 硬件配置

| 项目 | 配置 |
|------|------|
| MCU | STM32F407IGHx |
| CAN | CAN1（PD0/PD1），1 Mbps |
| 电机 | GM6020，拨码 ID = 1 |
| 控制周期 | 1kHz（TIM6 中断，Prescaler=84-1, Period=1000-1） |
| 看门狗 | IWDG，Prescaler=64，Reload=400（约 0.8s 超时） |

## 文件结构

```
C_board_src/
├── ControlTask.h     # 控制任务对外接口 + 模式枚举
├── ControlTask.cpp   # 正弦跟随控制 + 看门狗保护(核心实现)
├── Gm6020.h/.cpp     # GM6020 CAN 协议封装(角度/速度/电流/温度)
├── Pid.h/.cpp        # 位置式 PID(含输出限幅 + 抗积分饱和)
└── main.h / can.h / iwdg.h  # CubeMX 生成(需你自己工程中的)
```

## 可调参数

集中在 `ControlTask.cpp` 顶部：

```cpp
volatile float g_sine_freq = 0.5f;   // 正弦频率 Hz
volatile float g_vel_amp   = 2.0f;   // 速度幅值 rad/s
volatile float g_pos_amp   = 1.0f;   // 位置幅值 rad

static Pid g_velPid(1200.0f, 30.0f, 0.0f, 25000.0f, -25000.0f); // 速度环 PID
static Pid g_posPid(   8.0f,  0.0f, 0.35f,     6.0f,     -6.0f); // 位置环 PID
```

> 位置模式采用**串级控制**：位置环输出目标速度，速度环作为内环执行。

## 集成到你的工程

1. 把 `ControlTask.cpp` 加入编译（放在 `Core/Src` 或与 `Gm6020.cpp` 同目录）。
2. 在中断文件中转发回调：

```c
/* stm32f4xx_it.c 或 main.c */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
    if (htim->Instance == TIM6) {
        MainTask();          // 1kHz 控制入口
    }
}

void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan) {
    CAN_RxHeaderTypeDef rx;
    uint8_t data[8];
    if (hcan->Instance == CAN1) {
        HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &rx, data);
        if (rx.IDE == CAN_ID_STD) {
            CanFeedbackCallback(rx.StdId, data);  // 转发给控制任务
        }
    }
}
```

3. 在 `main.c` 所有 `MX_XXX_Init()` 之后调用一次初始化：

```c
/* main.c, 在 while(1) 之前 */
ControlTaskInit();
```

4. 运行期切换模式（可选）：

```c
CtrlSetMode(CTRL_MODE_VEL);   // 速度正弦跟随
CtrlSetMode(CTRL_MODE_POS);   // 位置正弦跟随
CtrlSetSine(0.5f, 2.0f);      // 设置频率与幅值
```

## 上传到 GitHub

```bash
git init
git add .
git commit -m "GM6020 sine velocity/position following with IWDG protection"
git branch -M main
git remote add origin https://github.com/<你的用户名>/<仓库名>.git
git push -u origin main
```

设为 public：在 GitHub 仓库页面 **Settings → General → 拉到底 Danger Zone → Change visibility → Make public**。

## 作者

`<你的名字>` — RoboMaster 嵌入式组
