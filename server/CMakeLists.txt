cmake_minimum_required(VERSION 3.14)
project(whisp LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# 全局 include 路径
include_directories(
    ${PROJECT_SOURCE_DIR}/src/common/include
    ${PROJECT_SOURCE_DIR}/src/util/include
    ${PROJECT_SOURCE_DIR}/src/network/include
    ${PROJECT_SOURCE_DIR}/src/core/message/include
    ${PROJECT_SOURCE_DIR}/src/core/session/include
)

# 添加子目录
add_subdirectory(src/common)
add_subdirectory(src/util)
add_subdirectory(src/network)
add_subdirectory(src/core/message)
add_subdirectory(src/core/session)

# 添加主程序
add_executable(whisp_server main.cpp)

# 链接子模块库
target_link_libraries(whisp_server
    common
    util
    network
    message
    session
)

# 第三方库
add_subdirectory(third_party/googletest)

# 测试
enable_testing()
add_subdirectory(test)