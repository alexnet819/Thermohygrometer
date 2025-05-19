cmake_minimum_required(VERSION 3.12)

# Library name (e.g., lcd1602_lib)
add_library(lcd1602_lib INTERFACE)

# Source files
target_sources(lcd1602_lib INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/src/lcd1602.cpp
)

# Include directories
target_include_directories(lcd1602_lib INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/inc
)

# Dependent libraries
target_link_libraries(lcd1602_lib INTERFACE
    pico_stdlib
    hardware_i2c
)
