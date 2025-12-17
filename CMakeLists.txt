cmake_minimum_required(VERSION 3.10)
project(gba_emu)

# Enable assembly language support
enable_language(ASM)

# C/C++ version
set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 11)

# Set assembly language standard if needed
if(CMAKE_ASM_COMPILER)
  set(CMAKE_ASM_STANDARD 11)
endif()

# Platform config
option(ENABLE_DEBUG "Enable debug build" OFF)
option(ENABLE_32BIT "Enable 32-bit build" OFF)
option(ENABLE_LTO "Enable link-time optimization" ON)
option(ENABLE_SYSMON "Enable LVGL System Monitor" OFF)
option(ENABLE_PROFILER "Enable profiling" OFF)

# Auto-detect Raspberry Pi
if(NOT DEFINED LV_USE_RPI AND CMAKE_SYSTEM_NAME MATCHES "Linux")
  if(EXISTS "/proc/device-tree/model")
    file(READ "/proc/device-tree/model" DEVICE_MODEL)
    if(DEVICE_MODEL MATCHES "Raspberry Pi")
      set(LV_USE_RPI ON)
      add_definitions(-DHAVE_WIRING_PI=1)
      set(WIRINGPI_LIBRARIES wiringPi)
      message(STATUS "Auto-detected Raspberry Pi: ${DEVICE_MODEL}")
    endif()
  endif()
endif()

if(ENABLE_32BIT)
  message(STATUS "32-bit build enabled")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -m32")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m32")
endif()

if(ENABLE_SYSMON)
  message(STATUS "LVGL System Monitor enabled")
  add_definitions(-DLV_USE_SYSMON=1)
endif()

if(ENABLE_PROFILER)
  message(STATUS "Profiling enabled")
  add_definitions(-DLV_USE_PROFILER=1)
endif()

if(ENABLE_DEBUG)
  message(STATUS "Debug build enabled")
  set(ENABLE_ASAN 1)
  set(CMAKE_BUILD_TYPE "Debug")
  add_definitions(-DDEBUG=1 -DLV_USE_ASSERT=1 -DLV_USE_STDLIB_CLIB=1)
else()
  message(STATUS "Release build enabled")
  add_definitions(-DDEBUG=0)
  set(CMAKE_BUILD_TYPE "Release")

  # Use string concatenation to avoid semicolon separation
  set(OPT_FLAGS "")
  string(
    CONCAT
      OPT_FLAGS
      "-Ofast "
      "-fwhole-program -fuse-linker-plugin " # Link-time optimization
      "-fdata-sections -ffunction-sections -Wl,--gc-sections " # Code size
      "-fno-stack-protector -fno-ident -fomit-frame-pointer " # Security
      "-falign-functions=1 -falign-jumps=1 -falign-loops=1 " # Alignment
      "-fno-unwind-tables -fno-asynchronous-unwind-tables -fno-unroll-loops " # Size
      "-fmerge-all-constants -fno-math-errno" # Math
  )

  if(ENABLE_LTO)
    message(STATUS "Link-time optimization enabled")
    set(OPT_FLAGS "${OPT_FLAGS} -flto=auto")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -flto")
  endif()

  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OPT_FLAGS}")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OPT_FLAGS}")

  # Set flags for assembly files
  if(CMAKE_ASM_COMPILER)
    set(CMAKE_ASM_FLAGS "${CMAKE_ASM_FLAGS} ${OPT_FLAGS}")
  endif()
endif()

# Common Platform
if(LV_USE_RPI)
  message(STATUS "Platform: Raspberry Pi - GamePi20")
  add_definitions(-DLV_USE_RPI=1 -DLV_USE_SDL=0 -DTHREADED_RENDERER=1)

  # FLAGS
  if(NOT ENABLE_DEBUG)
    # Auto-detect best floating point unit for Raspberry Pi
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "armv7" OR CMAKE_SYSTEM_PROCESSOR MATCHES
                                                 "aarch64")
      # Raspberry Pi 2/3/4 with ARMv7/ARMv8 support NEON
      set(RPI_BUILD_FLAGS
          "-mfpu=neon-vfpv4 -mfloat-abi=hard -march=armv7-a ${OPT_FLAGS}")
      add_definitions(-DHAVE_NEON=1)
      set(HAVE_NEON 1)
      message(
        STATUS "Enabled Raspberry Pi NEON optimizations: ${RPI_BUILD_FLAGS}")
    elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "armv6")
      # Raspberry Pi 1 with ARMv6 - use basic VFP
      set(RPI_BUILD_FLAGS "-mfpu=vfp ${OPT_FLAGS}")
      add_definitions(-DHAVE_NEON=0)
      message(
        STATUS "Enabled Raspberry Pi VFP optimizations: ${RPI_BUILD_FLAGS}")
    endif()

    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${RPI_BUILD_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${RPI_BUILD_FLAGS}")
  endif()

  set(ASOUND_LIBRARIES asound)

else()
  message(STATUS "Platform: UNIX - SDL2")
  add_definitions(-DLV_SCREEN_HOR_RES=320 -DLV_SCREEN_VER_RES=480
                  -DLV_USE_SDL=1 -DTHREADED_RENDERER=0 -DHAVE_NEON=0)

  add_definitions(-DUSE_SDL=1 -DHAVE_UNISTD_H=1 -DHAVE_FCNTL_H=1)
  find_package(SDL2 REQUIRED SDL2)
  include_directories(${SDL2_INCLUDE_DIRS} ${SDL2_INCLUDE_DIRS}/../)

  # Address Sanitizer
  if(ENABLE_ASAN)
    set(ASAN_FLAGS
        "-fsanitize=address -fsanitize=undefined -fno-omit-frame-pointer")
    message(STATUS "Enabled Address Sanitizer: ${ASAN_FLAGS}")
    add_definitions(${ASAN_FLAGS})
    set(CMAKE_EXE_LINKER_FLAGS ${CMAKE_EXE_LINKER_FLAGS} ${ASAN_FLAGS})
  endif()
endif()

# LVGL config
add_definitions(-DLV_USE_DEV_VERSION=1)

# vba-next config
add_definitions(
  -DFRONTEND_SUPPORTS_RGB565=1
  -DTILED_RENDERING=1
  -DSTATIC_LINKING=1
  -DUSE_TWEAKS=1
  -DUSE_FRAME_SKIP=1
  -Dretro_sleep=lv_port_sleep
  -DNDEBUG
  -D__LIBRETRO__
  -DINLINE=inline
  -DHAVE_STDINT_H
  -DHAVE_INTTYPES_H
  -DGBA_FRAME_SKIP="0"
  -DUSE_CACHE_PREFETCH=1)

# Include
set(VBA_NEXT_SOURCE_DIR ${PROJECT_SOURCE_DIR}/vba-next)

include_directories(${PROJECT_SOURCE_DIR} ${VBA_NEXT_SOURCE_DIR}/src
                    ${VBA_NEXT_SOURCE_DIR}/libretro-common/include)

# Sources porting
file(GLOB PORT_SOURCES "port/*.c")

if(LV_USE_RPI)
  file(GLOB PORT_RPI_SOURCES "port/rpi/*.c")
endif()

# gba emu
file(GLOB GBA_EMU_SOURCES "gba_emu/*.c*")

# vba-next
file(
  GLOB
  VBA_NEXT_SOURCE
  ${VBA_NEXT_SOURCE_DIR}/src/*.c
  ${VBA_NEXT_SOURCE_DIR}/src/*.cpp
  ${VBA_NEXT_SOURCE_DIR}/libretro/libretro.cpp
  ${VBA_NEXT_SOURCE_DIR}/libretro-common/rthreads/rsemaphore.c
  ${VBA_NEXT_SOURCE_DIR}/libretro-common/rthreads/rthreads.c
  ${VBA_NEXT_SOURCE_DIR}/libretro-common/compat/*.c
  ${VBA_NEXT_SOURCE_DIR}/libretro-common/encodings/*.c
  ${VBA_NEXT_SOURCE_DIR}/libretro-common/file/*.c
  ${VBA_NEXT_SOURCE_DIR}/libretro-common/streams/*.c
  ${VBA_NEXT_SOURCE_DIR}/libretro-common/string/*.c
  ${VBA_NEXT_SOURCE_DIR}/libretro-common/time/*.c)

# Add neon_memcpy.S only on RPI platforms with NEON support
if(HAVE_NEON)
  file(GLOB VBA_NEXT_NEON_SOURCE ${VBA_NEXT_SOURCE_DIR}/src/neon_memcpy.S)
  list(APPEND VBA_NEXT_SOURCE ${VBA_NEXT_NEON_SOURCE})
  message(STATUS "Included NEON assembly optimizations")
endif()

# ALL SOURCES
set(SOURCES ${PORT_SOURCES} ${PORT_RPI_SOURCES} ${GBA_EMU_SOURCES}
            ${GBA_PORT_SOURCES} ${VBA_NEXT_SOURCE})

# Subdirectory
add_subdirectory(${PROJECT_SOURCE_DIR}/lvgl ${PROJECT_BINARY_DIR}/build_lvgl)

# Executable
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
add_executable(gba_emu main.c ${SOURCES} ${INCLUDES})

# Link
target_link_libraries(
  gba_emu
  PRIVATE lvgl pthread
          # lvgl::examples lvgl::demos
          ${WIRINGPI_LIBRARIES} ${ASOUND_LIBRARIES} ${SDL2_LIBRARIES})

set_target_properties(gba_emu PROPERTIES RUNTIME_OUTPUT_DIRECTORY
                                         "${PROJECT_SOURCE_DIR}/build")
