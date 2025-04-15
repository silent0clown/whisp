# src/common/CMakeLists.txt

# 收集源文件,递归查找 src/common/src 中的 .cpp 文件
file(GLOB_RECURSE COMMON_SRC CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
)

# 添加库,构建为 common 静态库
add_library(common ${COMMON_SRC})

# 设置包含目录, 将 common/include 暴露为对外可用的头文件路径；
target_include_directories(common PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# 可以添加需要链接的第三方库（如 log 库等）
# target_link_libraries(common some_third_party_lib)