# ---------------------------------------------------------------------------
# RM_C_project 构建脚本（Windows，直接用 arm-none-eabi-gcc/g++，不依赖 make/ninja）
#
# 用法：
#   powershell -ExecutionPolicy Bypass -File build.ps1
#
# 产物：build\C_project.elf / .bin / .hex
# ---------------------------------------------------------------------------
param(
    [string]$CubePath    = "",
    [string]$ToolchainBin = ""
)

$ErrorActionPreference = "Continue"
$Root = $PSScriptRoot

# ---------- 定位 Cube 固件包 ----------
if (-not $CubePath) {
    $cands = @(
        (Join-Path $env:USERPROFILE "STM32Cube\Repository\STM32Cube_FW_F4_V1.28.3"),
        "D:\STM32Cube\Repository\STM32Cube_FW_F4_V1.28.3"
    )
    foreach ($c in $cands) {
        if (Test-Path (Join-Path $c "Drivers\STM32F4xx_HAL_Driver\Src\stm32f4xx_hal.c")) { $CubePath = $c; break }
    }
}
if (-not $CubePath) { throw "找不到 STM32Cube_FW_F4 固件包，请用 -CubePath 指定" }

# ---------- 定位工具链 ----------
if (-not $ToolchainBin) {
    $g = Get-Command arm-none-eabi-gcc -ErrorAction SilentlyContinue
    if ($g) { $ToolchainBin = Split-Path -Parent $g.Source }
    else {
        foreach ($c in @("D:\arm-toolchain\arm-gnu-toolchain-11.3.rel1-mingw-w64-i686-arm-none-eabi\bin",
                         "C:\Program Files (x86)\Arm GNU Toolchain arm-none-eabi\bin")) {
            if (Test-Path (Join-Path $c "arm-none-eabi-gcc.exe")) { $ToolchainBin = $c; break }
        }
    }
}
if (-not $ToolchainBin) { throw "找不到 arm-none-eabi-gcc，请用 -ToolchainBin 指定" }

$CC      = Join-Path $ToolchainBin "arm-none-eabi-gcc.exe"
$CXX     = Join-Path $ToolchainBin "arm-none-eabi-g++.exe"
$OBJCOPY = Join-Path $ToolchainBin "arm-none-eabi-objcopy.exe"
$SIZE    = Join-Path $ToolchainBin "arm-none-eabi-size.exe"

$Build = Join-Path $Root "build"
if (Test-Path $Build) { Remove-Item $Build -Recurse -Force -ErrorAction SilentlyContinue }
New-Item -ItemType Directory -Force -Path $Build | Out-Null

$arch = @("-mcpu=cortex-m4", "-mthumb", "-mfpu=fpv4-sp-d16", "-mfloat-abi=hard")
$defs = @("-DUSE_HAL_DRIVER", "-DSTM32F407xx")
$incs = @(
    "-I$(Join-Path $Root 'Core\Inc')",
    "-I$(Join-Path $Root 'C_board_src')",
    "-I$(Join-Path $CubePath 'Drivers\STM32F4xx_HAL_Driver\Inc')",
    "-I$(Join-Path $CubePath 'Drivers\STM32F4xx_HAL_Driver\Inc\Legacy')",
    "-I$(Join-Path $CubePath 'Drivers\CMSIS\Device\ST\STM32F4xx\Include')",
    "-I$(Join-Path $CubePath 'Drivers\CMSIS\Include')"
)
$cflags = @("-c", "-std=gnu11") + $arch + $defs + $incs + @("-ffunction-sections","-fdata-sections","-Wall","-O2","-g3")
$cxxflags = @("-c", "-std=gnu++17", "-fno-exceptions", "-fno-rtti") + $arch + $defs + $incs + @("-ffunction-sections","-fdata-sections","-Wall","-O2","-g3")

# ---------- 源文件 ----------
$cSrc = Get-ChildItem (Join-Path $Root "Core\Src\*.c") | ForEach-Object { $_.FullName }
$cppSrc = Get-ChildItem (Join-Path $Root "C_board_src\*.cpp") | ForEach-Object { $_.FullName }

$halModules = @("hal","hal_can","hal_cortex","hal_dma","hal_dma_ex","hal_flash",
                "hal_flash_ex","hal_gpio","hal_iwdg","hal_pwr","hal_pwr_ex",
                "hal_rcc","hal_rcc_ex","hal_tim","hal_tim_ex")
foreach ($m in $halModules) {
    $cSrc += (Join-Path $CubePath "Drivers\STM32F4xx_HAL_Driver\Src\stm32f4xx_$m.c")
}

$objs = @()
$fail = 0

Write-Host "=== 编译 C 源文件 ===" -ForegroundColor Cyan
foreach ($s in $cSrc) {
    if (-not (Test-Path $s)) { Write-Warning "缺少 $s"; $fail++; continue }
    $o = Join-Path $Build ((Split-Path $s -Leaf) + ".o")
    $out = & $CC @cflags $s -o $o 2>&1
    if (($LASTEXITCODE -ne 0) -or -not (Test-Path $o)) {
        Write-Host "失败: $s" -ForegroundColor Red
        $out | Select-Object -First 12 | ForEach-Object { Write-Host $_ }
        $fail++
    } else {
        $out | Where-Object { $_ -match "error|RM_C_project\\(Core|C_board_src)" -and $_ -match "warning" } |
            Select-Object -First 5 | ForEach-Object { Write-Host $_ -ForegroundColor Yellow }
    }
    $objs += $o
}

Write-Host "=== 编译 C++ 源文件 ===" -ForegroundColor Cyan
foreach ($s in $cppSrc) {
    $o = Join-Path $Build ((Split-Path $s -Leaf) + ".o")
    $out = & $CXX @cxxflags $s -o $o 2>&1
    if (($LASTEXITCODE -ne 0) -or -not (Test-Path $o)) {
        Write-Host "失败: $s" -ForegroundColor Red
        $out | Select-Object -First 20 | ForEach-Object { Write-Host $_ }
        $fail++
    } else {
        $out | Where-Object { $_ -match "warning" } | Select-Object -First 8 | ForEach-Object { Write-Host $_ -ForegroundColor Yellow }
    }
    $objs += $o
}

Write-Host "=== 汇编启动文件 ===" -ForegroundColor Cyan
$so = Join-Path $Build "startup.o"
& $CC -c @arch "-x" "assembler-with-cpp" (Join-Path $Root "startup\startup_stm32f407xx.s") -o $so 2>&1 | Out-Null
if ($LASTEXITCODE -ne 0) { Write-Host "启动文件编译失败" -ForegroundColor Red; $fail++ }
$objs += $so

if ($fail -gt 0) { throw "有 $fail 个文件编译失败" }

Write-Host "=== 链接 ===" -ForegroundColor Cyan
$elf = Join-Path $Build "C_project.elf"
$linkOut = & $CXX @objs -o $elf @arch `
    "-T$(Join-Path $Root 'ld\STM32F407IGHx_flash.ld')" `
    "-Wl,--gc-sections" `
    "-Wl,-Map=$(Join-Path $Build 'C_project.map')" `
    "--specs=nano.specs" "--specs=nosys.specs" 2>&1
$linkOut | Where-Object { $_ -notmatch "_close|_lseek|_read|_write|_sbrk|_fstat|_isatty|not implemented" } |
    Select-Object -First 15 | ForEach-Object { Write-Host $_ }
if (-not (Test-Path $elf)) { throw "链接失败" }

& $OBJCOPY -O binary $elf (Join-Path $Build "C_project.bin")
& $OBJCOPY -O ihex   $elf (Join-Path $Build "C_project.hex")

Write-Host ""
& $SIZE $elf
Write-Host ""
Write-Host "构建完成：" -ForegroundColor Green
Write-Host "  $elf"
Write-Host "  $(Join-Path $Build 'C_project.bin')"
Write-Host "  $(Join-Path $Build 'C_project.hex')"
