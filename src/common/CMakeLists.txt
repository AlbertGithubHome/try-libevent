# 这个目录可以包含一些共享的头文件和源文件
# 如果有源文件，可以添加为库
# add_library(common STATIC common.h)

ADD_DEFINITIONS(-D_THREAD_SAFE)

BUILD_STATIC_LIB(
    PROJECT_NAME "common"
    FOLDER_NAME "common"
    DEPENDENCIES_LIST
    LINK_LIBRARIES_LIST event_core event_extra
)