cmake_minimum_required(VERSION 3.20)

project(verushash
    VERSION 1.0.0
    DESCRIPTION "Standalone C ABI shared library for VerusHash"
    LANGUAGES C CXX)

add_library(verushash SHARED)
add_library(verushash::verushash ALIAS verushash)

target_sources(verushash PRIVATE
    verushash_lib.cpp
    crypto/verus_hash.cpp
    crypto/verus_clhash.cpp
    crypto/verus_clhash_portable.cpp
    crypto/haraka.c
    crypto/haraka_portable.c)

set_target_properties(verushash PROPERTIES
    C_STANDARD 99
    C_STANDARD_REQUIRED YES
    C_EXTENSIONS NO
    CXX_STANDARD 11
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
    POSITION_INDEPENDENT_CODE ON
    OUTPUT_NAME verushash
    PUBLIC_HEADER verushash_lib.h)

target_include_directories(verushash
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:include>)

target_compile_definitions(verushash PRIVATE VERUSHASH_LIB_BUILD)

string(TOLOWER "${CMAKE_SYSTEM_PROCESSOR};${CMAKE_GENERATOR_PLATFORM}" VERUSHASH_TARGET_ARCH)

if(MSVC)
    target_compile_options(verushash PRIVATE /O2 /EHsc /W3 /wd4146 /wd4244 /wd4267 /wd4309 /wd4101 /wd4334)
    target_compile_definitions(verushash PRIVATE _CRT_SECURE_NO_WARNINGS NOMINMAX)
    if(VERUSHASH_TARGET_ARCH MATCHES "x64|amd64|win32|x86|i386")
        target_compile_options(verushash PRIVATE /arch:AVX)
    endif()
else()
    target_compile_options(verushash PRIVATE -O3 -Wall -Wextra -Wno-unused-function -Wno-unused-variable -Wno-unused-parameter -Wno-sign-compare -Wno-strict-aliasing -Wno-maybe-uninitialized -Wno-uninitialized)
    if(VERUSHASH_TARGET_ARCH MATCHES "x86_64|amd64|i[3-6]86")
        target_compile_options(verushash PRIVATE -maes -mpclmul -mssse3 -mavx)
    endif()
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
        target_compile_options(verushash PRIVATE -fvisibility=hidden)
    endif()
endif()

include(GNUInstallDirs)

install(TARGETS verushash
    EXPORT verushashTargets
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(EXPORT verushashTargets
    NAMESPACE verushash::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/verushash)
