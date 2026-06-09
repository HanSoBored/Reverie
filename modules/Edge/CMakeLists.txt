cmake_minimum_required(VERSION 3.14)
project(edge C ASM)
set(CMAKE_C_STANDARD 23)

if(DEFINED ENV{ANDROID_NDK_HOME})
    set(ANDROID_NDK $ENV{ANDROID_NDK_HOME})
elseif(DEFINED ENV{NDK_HOME})
    set(ANDROID_NDK $ENV{NDK_HOME})
else()
    message(FATAL_ERROR "ANDROID_NDK_HOME not set")
endif()

if(NOT ANDROID_ABI)
    set(ANDROID_ABI "arm64-v8a")
endif()

if(ANDROID_ABI STREQUAL "arm64-v8a")
    set(SH_ARCH_DIR "arm64")
elseif(ANDROID_ABI STREQUAL "armeabi-v7a")
    set(SH_ARCH_DIR "arm")
else()
    message(FATAL_ERROR "ABI: ${ANDROID_ABI}")
endif()

set(MEMKIT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../deps/Android-Mem-Kit)
set(XDL_DIR ${MEMKIT_DIR}/deps/xdl/xdl)
set(SH_DIR ${MEMKIT_DIR}/deps/shadowhook/shadowhook)

include_directories(
    ${MEMKIT_DIR}/include
    ${MEMKIT_DIR}/src
    ${XDL_DIR}/src/main/cpp/include
    ${SH_DIR}/src/main/cpp/include
    ${SH_DIR}/src/main/cpp
    ${SH_DIR}/src/main/cpp/common
    ${SH_DIR}/src/main/cpp/arch/${SH_ARCH_DIR}
    ${SH_DIR}/src/main/cpp/third_party/bsd
    ${SH_DIR}/src/main/cpp/third_party/xdl
    ${SH_DIR}/src/main/cpp/third_party/lss
    ${CMAKE_CURRENT_BINARY_DIR}/gen
)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wno-macro-redefined -fdiagnostics-format=clang")

add_library(xdl STATIC
    ${XDL_DIR}/src/main/cpp/xdl.c
    ${XDL_DIR}/src/main/cpp/xdl_iterate.c
    ${XDL_DIR}/src/main/cpp/xdl_linker.c
    ${XDL_DIR}/src/main/cpp/xdl_lzma.c
    ${XDL_DIR}/src/main/cpp/xdl_util.c
)
target_compile_definitions(xdl PRIVATE __ANDROID__API__=35)

add_library(shadowhook STATIC
    ${SH_DIR}/src/main/cpp/arch/${SH_ARCH_DIR}/sh_a64.c
    ${SH_DIR}/src/main/cpp/arch/${SH_ARCH_DIR}/sh_inst.c
    ${SH_DIR}/src/main/cpp/arch/${SH_ARCH_DIR}/sh_glue.S
    ${SH_DIR}/src/main/cpp/common/bytesig.c
    ${SH_DIR}/src/main/cpp/common/sh_errno.c
    ${SH_DIR}/src/main/cpp/common/sh_log.c
    ${SH_DIR}/src/main/cpp/common/sh_ref.c
    ${SH_DIR}/src/main/cpp/common/sh_trampo.c
    ${SH_DIR}/src/main/cpp/common/sh_util.c
    ${SH_DIR}/src/main/cpp/sh_elf.c
    ${SH_DIR}/src/main/cpp/sh_enter.c
    ${SH_DIR}/src/main/cpp/sh_hub.c
    ${SH_DIR}/src/main/cpp/sh_island.c
    ${SH_DIR}/src/main/cpp/sh_jni.c
    ${SH_DIR}/src/main/cpp/sh_linker.c
    ${SH_DIR}/src/main/cpp/sh_recorder.c
    ${SH_DIR}/src/main/cpp/sh_safe.c
    ${SH_DIR}/src/main/cpp/sh_switch.c
    ${SH_DIR}/src/main/cpp/sh_task.c
    ${SH_DIR}/src/main/cpp/sh_xdl.c
    ${SH_DIR}/src/main/cpp/shadowhook.c
    ${SH_DIR}/src/main/cpp/third_party/xdl/xdl.c
    ${SH_DIR}/src/main/cpp/third_party/xdl/xdl_iterate.c
    ${SH_DIR}/src/main/cpp/third_party/xdl/xdl_linker.c
    ${SH_DIR}/src/main/cpp/third_party/xdl/xdl_lzma.c
    ${SH_DIR}/src/main/cpp/third_party/xdl/xdl_util.c
)
target_compile_definitions(shadowhook PRIVATE __ANDROID__API__=35)

add_library(shadowhook_nothing SHARED
    ${SH_DIR}/src/main/cpp/nothing/sh_nothing.c
)
target_compile_options(shadowhook_nothing PRIVATE
    -Oz -Weverything -Werror -Wno-missing-include-dirs -fno-ident -fno-unwind-tables
    -fno-asynchronous-unwind-tables
)
set(_shl
    "-Wl,--strip-all" "-Wl,--as-needed" "-Wl,--hash-style=sysv"
    "-Wl,--build-id=none" "-Wl,-nostdlib" "-nostartfiles"
    "-nodefaultlibs" "-Wl,-soname,libshadowhook_nothing.so"
)
if(ANDROID_ABI STREQUAL "arm64-v8a")
    list(APPEND _shl "-Wl,-z,max-page-size=16384")
endif()
target_link_options(shadowhook_nothing PRIVATE ${_shl})
unset(_shl)

set(NOTHING_EMBED_H_PATH "${CMAKE_CURRENT_BINARY_DIR}/gen/nothing_embed.h")
add_custom_command(
    OUTPUT ${NOTHING_EMBED_H_PATH}
    COMMAND ${CMAKE_COMMAND}
        -DINPUT_FILE="$<TARGET_FILE:shadowhook_nothing>"
        -DOUTPUT_FILE=${NOTHING_EMBED_H_PATH}
        -P ${MEMKIT_DIR}/cmake/gen_nothing_header.cmake
    DEPENDS shadowhook_nothing
    COMMENT "Generating nothing_embed.h from libshadowhook_nothing.so"
)
add_custom_target(generate_nothing_embed DEPENDS ${NOTHING_EMBED_H_PATH})

add_library(edge SHARED
    ${MEMKIT_DIR}/src/memory.c
    ${MEMKIT_DIR}/src/hooking.c
    ${MEMKIT_DIR}/src/hooking_flags.c
    ${MEMKIT_DIR}/src/intercept.c
    ${MEMKIT_DIR}/src/records.c
    ${MEMKIT_DIR}/src/runtime_config.c
    ${MEMKIT_DIR}/src/dl_callbacks.c
    ${MEMKIT_DIR}/src/il2cpp.c
    ${MEMKIT_DIR}/src/il2cpp_safe.c
    ${MEMKIT_DIR}/src/xdl_wrapper.c
    ${MEMKIT_DIR}/src/nothing_path.c
    ${MEMKIT_DIR}/src/shadowhook_override.c
    src/main.c
)

if(TARGET generate_nothing_embed)
    add_dependencies(edge generate_nothing_embed)
endif()

target_link_libraries(edge xdl shadowhook log android)

target_link_options(edge PRIVATE
    "LINKER:--wrap,dlopen"
    "LINKER:--wrap,sh_linker_init"
)

set_target_properties(edge PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/lib"
)
