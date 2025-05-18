cmake_minimum_required(VERSION 3.12)

set(PICO_SDK_FETCH_FROM_GIT on)

# Pico SDKのパスを環境変数から取得
if (NOT DEFINED ENV{PICO_SDK_PATH})
    message(FATAL_ERROR "PICO_SDK_PATH environment variable is not set")
endif()

# コンパイラの明示的な指定
set(CMAKE_C_COMPILER arm-none-eabi-gcc)
set(CMAKE_CXX_COMPILER arm-none-eabi-g++)

# C17標準を設定
set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
# C++20標準を設定
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# クロスコンパイル設定を有効化
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)
# Pico SDKのプラットフォームとコンパイラを指定
# raspberry pi pico / wはrp2040、raspberry pi pico2 / wはRP2350
set(PICO_PLATFORM rp2350)

# コンパイラの指定
# pico_arm_cortex_m0plus_gccはRP2040用(pico用)
# pico_arm_cortex_m33_gccはRP2350用(pico2用)
# set(PICO_COMPILER pico_arm_cortex_m0plus_gcc)
set(PICO_COMPILER pico_arm_cortex_m33_gcc)

# 以下が対応しているボード名一覧
# raspberry pi pico = pico
# raspberry pi pico w = pico_w
# raspberry pi pico2 = pico2
# raspberry pi pico2 w = pico2_w
set(PICO_BOARD pico2_w)

# Pico SDKを初期化する（project宣言の前に行う必要があります）
include($ENV{PICO_SDK_PATH}/pico_sdk_init.cmake)

# プロジェクト定義
project(Thermohygrometer C CXX ASM)

# Pico SDKの初期化
# Picoを使用するために必要
pico_sdk_init()

add_subdirectory(dht11_22)
add_subdirectory(lcd1602)

add_executable(thermohygrometer
        main.cpp
        )

# pull in common dependencies and additional i2c hardware support
target_link_libraries(thermohygrometer 
                      pico_stdlib 
                      hardware_i2c
                      hardware_gpio  # DHTセンサーで使用
                      dht_lib        # dht11_22サブディレクトリのライブラリ
                      lcd1602_lib    # lcd1602サブディレクトリのライブラリ
                      )

# USBシリアル出力を有効化
pico_enable_stdio_usb(thermohygrometer 1)
pico_enable_stdio_uart(thermohygrometer 0)

# UF2ファイル生成を有効化
pico_add_extra_outputs(thermohygrometer)