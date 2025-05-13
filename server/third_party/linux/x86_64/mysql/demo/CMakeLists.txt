cmake_minimum_required(VERSION 3.10)
project(mysql_demo)

# 设置头文件和库路径
include_directories(${CMAKE_SOURCE_DIR}/../include)
link_directories(${CMAKE_SOURCE_DIR}/../lib)

# 设置输出文件
add_executable(mysql_demo mysql_demo.cpp)

# 链接 MySQL 客户端库
target_link_libraries(mysql_demo mysqlclient)

# 防止找不到符号（可选）
set_target_properties(mysql_demo PROPERTIES
    LINK_FLAGS "-Wl,-rpath=${CMAKE_SOURCE_DIR}/third_party/mysql/lib"
)
