add_executable(server server.cpp)

# 链接libevent库
target_link_libraries(server PRIVATE event_core event_extra)
