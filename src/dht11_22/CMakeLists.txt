cmake_minimum_required(VERSION 3.12)

# Library name (e.g., dht_lib)
add_library(dht_lib INTERFACE)

# Source files
target_sources(dht_lib INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/src/dht.cpp
)

# Include directories
target_include_directories(dht_lib INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/inc
)

# Dependent libraries
target_link_libraries(dht_lib INTERFACE
    pico_stdlib
    hardware_gpio
)
