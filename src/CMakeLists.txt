cmake_minimum_required(VERSION 3.12)

set(PICO_SDK_FETCH_FROM_GIT on)

# Get Pico SDK path from environment variable
if (NOT DEFINED ENV{PICO_SDK_PATH})
    message(FATAL_ERROR "PICO_SDK_PATH environment variable is not set")
endif()

# Explicitly specify compilers
set(CMAKE_C_COMPILER arm-none-eabi-gcc)
set(CMAKE_CXX_COMPILER arm-none-eabi-g++)

# Set C17 standard
set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
# Set C++20 standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable cross-compilation settings
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)
# Specify Pico SDK platform and compiler
# raspberry pi pico / w is [rp2040], raspberry pi pico2 / w is [RP2350]
set(PICO_PLATFORM rp2350)

# Specify compiler
# [pico_arm_cortex_m0plus_gcc] is for RP2040 (for pico)
# [pico_arm_cortex_m33_gcc] is for RP2350 (for pico2)
# set(PICO_COMPILER pico_arm_cortex_m0plus_gcc)
set(PICO_COMPILER pico_arm_cortex_m33_gcc)

# List of supported board names
# raspberry pi pico = pico
# raspberry pi pico w = pico_w
# raspberry pi pico2 = pico2
# raspberry pi pico2 w = pico2_w
set(PICO_BOARD pico2_w)

# Initialize Pico SDK (must be done before project declaration)
include($ENV{PICO_SDK_PATH}/pico_sdk_init.cmake)

# Project definition
project(Thermohygrometer C CXX ASM)

# Initialize Pico SDK
# Required to use Pico
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
                      hardware_gpio  # Used by DHT sensor
                      dht_lib        # Library from dht11_22 subdirectory
                      lcd1602_lib    # Library from lcd1602 subdirectory
                      )

# Enable USB serial output
pico_enable_stdio_usb(thermohygrometer 1)
pico_enable_stdio_uart(thermohygrometer 0)

# Enable UF2 file generation
pico_add_extra_outputs(thermohygrometer)