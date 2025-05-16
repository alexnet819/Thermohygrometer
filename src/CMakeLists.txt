cmake_minimum_required(VERSION 3.12)

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
# Pcio2を使用する場合にはRP2350を指定
set(PICO_PLATFORM rp2040)
set(PICO_COMPILER pico_arm_gcc)

# Pico W向けの設定（必要に応じてpico_wからpicoに変更可能）
# Pico 2を使用する場合はpico2を指定
# 見てもわからないことが多いですが、sdkのgithubのreadmeに書いてあるのでそれを参照するといいです
# https://github.com/raspberrypi/pico-sdk
set(PICO_BOARD pico_w) # pico_w から pico に変更 (または必要に応じて pico_w のまま)

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