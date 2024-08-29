cmake_minimum_required(VERSION 3.10)
project(try-libevent)

# 设置C++标准
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 添加3rdparty目录
# add_subdirectory(3rdparty/libevent)

# 添加子目录
add_subdirectory(src/client)
add_subdirectory(src/server)
add_subdirectory(src/common)
