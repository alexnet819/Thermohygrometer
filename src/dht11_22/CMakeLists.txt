cmake_minimum_required(VERSION 3.12)

# ライブラリ名 (例: dht_lib)
add_library(dht_lib INTERFACE)

# ソースファイル
target_sources(dht_lib INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/src/dht.cpp
)

# インクルードディレクトリ
target_include_directories(dht_lib INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/inc
)

# 依存ライブラリ
target_link_libraries(dht_lib INTERFACE
    pico_stdlib
    hardware_gpio
)
