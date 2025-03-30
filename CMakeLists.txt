cmake_minimum_required(VERSION 3.20)
project(sk82recomp LANGUAGES C CXX)

set(TARGET_NAME sk82recomp)

file(GLOB_RECURSE SOURCE_FILES "${CMAKE_SOURCE_DIR}/src/ppc/ppc_recomp.*.cpp")

set(EXTERNAL_ROOT ${CMAKE_SOURCE_DIR}/external)

# include directories
include_directories(${CMAKE_SOURCE_DIR}/src)
include_directories(${CMAKE_SOURCE_DIR}/external)

# COMPILE O1HEAP
add_library(o1heap STATIC ${EXTERNAL_ROOT}/o1heap/o1heap.c)
set_property(TARGET o1heap PROPERTY LINKER_LANGUAGE C)
target_compile_options(o1heap PRIVATE -m64)
set_property(TARGET o1heap PROPERTY C_STANDARD 99)

# COMPILE SDL
add_subdirectory(${EXTERNAL_ROOT})

add_executable(${TARGET_NAME} 
    src/main.cpp

    src/kernel/memory.h
    src/kernel/memory.cpp
    
    src/kernel/function.h
    src/kernel/ppc_local.h

    src/kernel/xdm.cpp
    src/kernel/xdm.h

    src/graphics/video.cpp
    src/graphics/video.h

    src/cpu/guest_thread.cpp
    src/cpu/guest_thread.h
    src/cpu/mutex.h

    # hooks
    src/audio/audio_hooks.cpp

    src/graphics/video_hooks.cpp
    src/graphics/video_hooks.h

    src/kernel/hooks/hooks.h
    src/kernel/hooks/xfilesystem.cpp
    src/kernel/hooks/xgpu.cpp
    src/kernel/hooks/xkernel.cpp
    src/kernel/hooks/xam.cpp
    src/kernel/hooks/xmem.cpp
    src/kernel/hooks/xthreading.cpp

    src/input/input_hooks.cpp

    src/loader.cpp
    src/loader.h

    src/xex.h

    src/ppc/ppc_func_mapping.cpp

    ${SOURCE_FILES} 
)

target_link_libraries(${TARGET_NAME} 
    SDL3::SDL3
    d3d9 
    o1heap
)

set_property(TARGET ${TARGET_NAME} PROPERTY CXX_STANDARD 23)
set_property(TARGET ${TARGET_NAME} PROPERTY CXX_STANDARD_REQUIRED ON)

target_compile_options(${TARGET_NAME} PRIVATE -msse4.1 -msse4.2 -mavx -m64)
