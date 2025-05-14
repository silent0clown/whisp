# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB_RECURSE TEST_SOURCES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
)

include_directories(${THIRD_PARTY_PATH}/googletest/include)
link_directories(${THIRD_PARTY_PATH}/googletest/lib)

add_executable(test_whisp_proc ${TEST_SOURCES})
target_include_directories(test_whisp_proc PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)
target_link_libraries(test_whisp_proc PRIVATE
    gtest
)


# 启用测试
enable_testing()
add_test(NAME test_whisp_proc COMMAND TalkoTests)

install(TARGETS test_whisp_proc DESTINATION test)