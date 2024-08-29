add_executable(client client.cpp)

# 链接libevent库
target_link_libraries(client PRIVATE event_core event_extra)
