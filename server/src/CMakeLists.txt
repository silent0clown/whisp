# src/CMakeLists.txt
# ./src/CMakeLists.txt      # 可执行文件定义和子模块组织
# 添加子模块
# add_subdirectory(network)
add_subdirectory(common)
add_subdirectory(db)
add_subdirectory(network)
add_subdirectory(core)
# add_subdirectory(infra)
add_subdirectory(util)

# 生成可执行文件
add_executable(whisp_server_proc main.cpp)

# 连接库
target_link_libraries(whisp_server_proc PRIVATE
        # network 
        common
        db
        network
        core
        # infra
        util
        )

# 安装规则（可选）
install(TARGETS whisp_server_proc
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    # ARCHIVE DESTINATION lib
)