cmake_minimum_required(VERSION 3.10)
project(gba_emu)

# C/C++ version
set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 11)

# Platform config

set(ENABLE_ASAN 0)
set(CMAKE_BUILD_TYPE "Debug")
# set(CMAKE_BUILD_TYPE "Release")

#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -m32")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m32")

## Common
# Platform
if (LV_USE_RPI)
message(STATUS "Platform: Raspberry Pi - GamePi20")
add_definitions(
        -DLV_USE_RPI=1
        -DLV_USE_SDL=0
)
else()
message(STATUS "Platform: UNIX - SDL2")
add_definitions(
        -DLV_SCREEN_HOR_RES=320
        -DLV_SCREEN_VER_RES=480
        -DLV_USE_SDL=1
)

add_definitions(-DUSE_SDL=1 -DHAVE_UNISTD_H=1 -DHAVE_FCNTL_H=1)
find_package(SDL2 REQUIRED SDL2)
include_directories(
        ${SDL2_INCLUDE_DIRS}
        ${SDL2_INCLUDE_DIRS}/../
)

# Address Sanitizer
if (ENABLE_ASAN)
        set(ASAN_FLAGS "-fsanitize=address -fsanitize=undefined -fno-omit-frame-pointer")
        add_definitions(${ASAN_FLAGS})
        set(CMAKE_EXE_LINKER_FLAGS ${CMAKE_EXE_LINKER_FLAGS} ${ASAN_FLAGS})
endif()
endif()

# LVGL config
add_definitions(
        -DLV_USE_DEV_VERSION=1
)

# vba-next config
add_definitions(
        -DDEBUG=0
        -DFRONTEND_SUPPORTS_RGB565=1
        -DTILED_RENDERING=0
        -DSTATIC_LINKING=0
        -DUSE_TWEAKS=0
        -DUSE_THREADED_RENDERER=0
        -DUSE_FRAME_SKIP=1
        -DHAVE_NEON=0

        -DNDEBUG
        -D__LIBRETRO__
        -DINLINE=inline
        -DHAVE_STDINT_H
        -DHAVE_INTTYPES_H

        -DGBA_FRAME_SKIP="0"
)

# Include
set(VBA_NEXT_SOURCE_DIR ${PROJECT_SOURCE_DIR}/vba-next)

include_directories(
        ${PROJECT_SOURCE_DIR}
        ${VBA_NEXT_SOURCE_DIR}/src
        ${VBA_NEXT_SOURCE_DIR}/libretro-common/include
)

# Sources
## porting
file(GLOB PORT_SOURCES "port/*.c")

if (LV_USE_RPI)
file(GLOB PORT_RPI_SOURCES "port/rpi/*.c")
set(RPI_LINK_LIBS wiringPi pthread asound)
endif ()

## gba emu
file(GLOB GBA_EMU_SOURCES "gba_emu/*.c*")

## vba-next
file(GLOB VBA_NEXT_SOURCE
        ${VBA_NEXT_SOURCE_DIR}/src/*.c
        ${VBA_NEXT_SOURCE_DIR}/src/*.cpp
        ${VBA_NEXT_SOURCE_DIR}/libretro/libretro.cpp
        ${VBA_NEXT_SOURCE_DIR}/libretro-common/compat/*.c
        ${VBA_NEXT_SOURCE_DIR}/libretro-common/encodings/*.c
        ${VBA_NEXT_SOURCE_DIR}/libretro-common/file/*.c
        ${VBA_NEXT_SOURCE_DIR}/libretro-common/streams/*.c
        ${VBA_NEXT_SOURCE_DIR}/libretro-common/string/*.c
        ${VBA_NEXT_SOURCE_DIR}/libretro-common/time/*.c
)

## ALL SOURCES
set(SOURCES
        ${PORT_SOURCES}
        ${PORT_RPI_SOURCES}
        ${GBA_EMU_SOURCES}
        ${GBA_PORT_SOURCES}
        ${VBA_NEXT_SOURCE}
)

# Subdirectory
add_subdirectory(${PROJECT_SOURCE_DIR}/lvgl ${PROJECT_BINARY_DIR}/build_lvgl)

# Executable
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
add_executable(gba_emu main.c ${SOURCES} ${INCLUDES})

# Link
target_link_libraries(
        gba_emu PRIVATE
        lvgl
        # lvgl::examples
        # lvgl::demos
        ${RPI_LINK_LIBS}
        ${SDL2_LIBRARIES}
)

set_target_properties(
        gba_emu
        PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/build"
)
