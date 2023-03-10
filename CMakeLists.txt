cmake_minimum_required(VERSION 3.5)

project(nds)

set(CMAKE_BUILD_TYPE DEBUG)

set(SOURCES src/main.cpp
            src/core/bus.cpp
            src/core/arm9/instr_decoding.cpp
            src/core/arm9/arm9.cpp
            src/core/arm9/cp15.cpp
            src/core/arm7/arm7.cpp
			src/core/gpu/gpu.cpp
			src/core/spi/rtc.cpp
			src/core/spi/cart.cpp
			src/core/spi/firmware.cpp)

find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})
			
add_executable(nds ${SOURCES})

target_link_libraries(nds ${SDL2_LIBRARIES})

target_include_directories(nds PRIVATE ${CMAKE_SOURCE_DIR})

set_property(TARGET nds PROPERTY CXX_STANDARD 20)