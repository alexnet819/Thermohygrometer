cmake_minimum_required(VERSION 3.12)

# Library name (e.g., dht_lib)
add_library(dht_lib STATIC)

# Source files
target_sources(dht_lib PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/src/dht.cpp
)

# Include directories
# PUBLIC so that targets linking dht_lib can find its headers
target_include_directories(dht_lib PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/inc
)

# Dependent libraries
# PUBLIC so that link dependencies and include paths from these are propagated
target_link_libraries(dht_lib PUBLIC
    pico_stdlib
    hardware_gpio
)
