cmake_minimum_required(VERSION 3.12)

# ライブラリ名 (例: lcd1602_lib)
add_library(lcd1602_lib INTERFACE)

# ソースファイル
target_sources(lcd1602_lib INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/src/lcd1602.cpp
)

# インクルードディレクトリ
target_include_directories(lcd1602_lib INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/inc
)

# 依存ライブラリ
target_link_libraries(lcd1602_lib INTERFACE
    pico_stdlib
    hardware_i2c
)
