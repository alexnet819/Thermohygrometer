cmake_minimum_required(VERSION 3.12)

# Library name (e.g., lcd1602_lib)
add_library(lcd1602_lib STATIC)

# Source files
target_sources(lcd1602_lib PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/src/lcd1602.cpp
)

# Include directories
# PUBLIC so that targets linking lcd1602_lib can find its headers
target_include_directories(lcd1602_lib PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/inc
)

# Dependent libraries
# PUBLIC so that link dependencies and include paths from these are propagated
target_link_libraries(lcd1602_lib PUBLIC
    pico_stdlib
    hardware_i2c
)
