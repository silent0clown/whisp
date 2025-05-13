# src/util/CMakeLists.txt

# 收集源文件,递归查找 src/util/src 中的 .cpp 文件
file(GLOB_RECURSE UTIL_SRC CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
)

# 添加库,构建为 util 静态库
add_library(util ${UTIL_SRC})

# 设置包含目录, 将 util/include 暴露为对外可用的头文件路径；
target_include_directories(util PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)


add_library(zlib STATIC IMPORTED)
# 设置 zlib 的包含目录
# message(STATUS "zlib include dir: ${THIRD_PARTY_DIR}/zlib/include")
set_target_properties(zlib PROPERTIES IMPORTED_LOCATION ${THIRD_PARTY_PATH}/zlib/lib/libz.a)

target_link_libraries(util PUBLIC
    zlib
)