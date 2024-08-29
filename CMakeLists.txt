cmake_minimum_required(VERSION 3.10)
project(try-libevent)

#--------------------------------------------
# Project setting
#--------------------------------------------
INCLUDE(cmake/buildobject.cmake)

# 设置C++标准
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
SET(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "")    # generate compile_commands.json


SET(SLN_DIR ${PROJECT_SOURCE_DIR})

IF(CMAKE_BUILD_TYPE STREQUAL "Debug")
    SET(BUILDS_DIR ${PROJECT_SOURCE_DIR}/bin/debug)
ELSE()
    SET(BUILDS_DIR ${PROJECT_SOURCE_DIR}/bin/release)
ENDIF()

INCLUDE_DIRECTORIES(
    ${SLN_DIR}
    ${SLN_DIR}/3rdparty/
    ${SLN_DIR}/3rdparty/protobuf/
    ${SLN_DIR}/3rdparty/protobuf/src/
    ${SLN_DIR}/3rdparty/libevent/
    ${SLN_DIR}/3rdparty/libevent/include/
    ${SLN_DIR}/3rdparty/libevent/compat/
    ${SLN_DIR}/3rdparty/openssl/
    ${SLN_DIR}/3rdparty/openssl/include/
    ${SLN_DIR}/src/
)

IF(UNIX)
    SET(NO_BUILDIN_OPTIONS "-fno-builtin-malloc -fno-builtin-calloc -fno-builtin-realloc -fno-builtin-free")

    IF(CMAKE_BUILD_TYPE STREQUAL "Debug")
        SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -Wall -g -ggdb -DDEBUG -fcoroutines -rdynamic")
    ELSE()
        SET(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -rdynamic ${NO_BUILDIN_OPTIONS}")
        SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -Wall -fcoroutines -rdynamic ${NO_BUILDIN_OPTIONS}")
    ENDIF()

    SET(LDFLAGS "$ENV{LDFLAGS} -stdlib=libc++")
ELSE()
    SET(CMAKE_CXX_FLAGS "/std:c++20 /permissive- /Zc:__cplusplus /Zc:preprocessor -D_X64 -D_WINDOWS /EHsc" )
    INCLUDE_DIRECTORIES(${SLN_DIR}/3rdparty/libevent/WIN32-Code/)
ENDIF()

IF(CMAKE_BUILD_TYPE STREQUAL "Debug")
    LINK_DIRECTORIES(
        ${SLN_DIR}/3rdparty/lib/
        ${SLN_DIR}/bin/debug/
    )
ELSE()
    LINK_DIRECTORIES(
        ${SLN_DIR}/3rdparty/lib/
        ${SLN_DIR}/bin/release/
    )
ENDIF()

# 添加3rdparty目录
# add_subdirectory(3rdparty/libevent)

# 添加子目录
add_subdirectory(src)
