# ./CMakeLists.txt          # 全局配置，包含第三方库、编译选项等
cmake_minimum_required(VERSION 3.15)
project(whisp_server LANGUAGES C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    # 添加警告级别和警告视为错误选项（适用于GCC和Clang）
    add_compile_options(
        -Wall            # 开启大多数常规警告
        -Wextra          # 更多额外警告
        -Wpedantic       # 遵循严格的语言标准
        -Wconversion     # 隐式类型转换警告
        -Wsign-conversion # 有符号/无符号转换警告
        -Wshadow         # 局部变量遮蔽警告
        -Wnon-virtual-dtor # 非虚析构函数警告（对类层次结构有帮助）
        -Wold-style-cast # C 风格类型转换警告（推荐用 static_cast 等）
        -Wcast-align     # 不当内存对齐警告
        -Wunused         # 未使用变量等
    )
    # 可选：将警告当作错误（开发阶段推荐，CI上建议开启）
    # add_compile_options(-Werror)
elseif (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    add_compile_options(
        /W4              # 开启 MSVC 警告等级 4
        /permissive-     # 更接近 C++ 标准行为
        /wd4996          # 禁用某些烦人的弃用警告
    )
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON -DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++)   # 开启后，其生成的文件compile_commands.json，包含所有编译单元所执行的指令
# 设置PROJECT_ROOT_PATH
set(PROJ_ROOT_PATH "${CMAKE_SOURCE_DIR}")
message(STATUS "Project root path: ${PROJ_ROOT_PATH}")
add_compile_definitions(PROJ_PATH="${PROJ_ROOT_PATH}")

# 设置编译操作系统(SYSTEM)和芯片架构信息(Architecture)
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(SYSTEM "linux")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(SYSTEM "windows")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    set(SYSTEM "macOS")
else()
    set(SYSTEM "unknown")
endif()


# 获取 CPU 架构
if(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|AMD64")
    set(ARCHITECTURE "x86_64")
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "i686|i386")
    set(ARCHITECTURE "x86")
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64|arm64")
    set(ARCHITECTURE "arm64")
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "armv7l")
    set(ARCHITECTURE "ARMv7")
else()
    set(ARCHITECTURE "unknown")
endif()

# 输出编译环境信息
message(STATUS "System info: ${SYSTEM} on ${ARCHITECTURE}")

# 设置默认安装路径
set(CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR}/output)

# 判断编译类型，并设置优化等级
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(STATUS "Debug模式：启用调试符号")
    # Debug模式下不做优化，启用调试符号
    set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
    message(STATUS "Release模式：启用优化")
    # Release模式下启用优化
    set(CMAKE_CXX_FLAGS_RELEASE "-g -O2")
else()
    message(STATUS "未知的编译类型: ${CMAKE_BUILD_TYPE}")
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_definitions(-DWHISP_BUILD_MODE=0)
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
    add_definitions(-DWHISP_BUILD_MODE=1)
elseif(CMAKE_BUILD_TYPE STREQUAL "Test")
    add_definitions(-DWHISP_BUILD_MODE=2)
else()
    message(WARNING "Unknown build type: ${CMAKE_BUILD_TYPE}, defaulting to DEBUG")
    add_definitions(-DWHISP_BUILD_MODE=0)
endif()


# 添加三方库头文件路径
set(THIRD_PARTY_PATH ${PROJ_ROOT_PATH}/third_party/${SYSTEM}/${ARCHITECTURE})
message(STATUS "THIRD_PARTY_PATH: ${THIRD_PARTY_PATH}")

# 添加主项目子文件夹
add_subdirectory(src)

# 如果启用测试，添加测试子目录
option(BUILD_TESTS "Build tests" OFF)
if (BUILD_TESTS)
    add_subdirectory(test)
endif()

# 安装规则
# install(TARGETS whisp_server_proc DESTINATION bin)

# # 全局 include 路径
# include_directories(
#     ${PROJECT_SOURCE_DIR}/src/common/include
#     ${PROJECT_SOURCE_DIR}/src/util/include
#     ${PROJECT_SOURCE_DIR}/src/network/include
#     ${PROJECT_SOURCE_DIR}/src/core/message/include
#     ${PROJECT_SOURCE_DIR}/src/core/session/include
# )

# # 添加子目录
# add_subdirectory(src/common)
# add_subdirectory(src/util)
# add_subdirectory(src/network)
# add_subdirectory(src/core/message)
# add_subdirectory(src/core/session)

# # 添加主程序
# add_executable(whisp_server main.cpp)

# # 链接子模块库
# target_link_libraries(whisp_server
#     common
#     util
#     network
#     message
#     session
# )

# # 第三方库
# add_subdirectory(third_party/googletest)

# # 测试
# enable_testing()
# add_subdirectory(test)