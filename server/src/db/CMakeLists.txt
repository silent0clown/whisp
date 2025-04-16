# src/db/CMakeLists.txt

file(GLOB_RECURSE DB_SRC CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
)

add_library(db ${DB_SRC})

target_include_directories(db PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

message(STATUS "DB BUILD THIRD_PARTY_PATH: ${THIRD_PARTY_PATH}")


# 这里需要使用PUBLIC

# PRIVATE	只给当前 target 自己使用
# PUBLIC	当前 target 和所有链接它的 target 都能用
# INTERFACE	只有链接当前 target 的目标才能用，自己不用（很少用）

# main → db → mysqlconn headers 如果使用private，db能编译通过，但是 main 引用了 #include <mysqlx/xdevapi.h> → ❌ 编译失败（路径没传过去）
target_include_directories(db SYSTEM PUBLIC
    ${THIRD_PARTY_PATH}/openssl/include
    ${THIRD_PARTY_PATH}/mysqlconn/include
)



add_library(crypto SHARED IMPORTED)
add_library(ssl SHARED IMPORTED)
add_library(mysqlconn8 SHARED IMPORTED)
set_target_properties(crypto PROPERTIES IMPORTED_LOCATION ${THIRD_PARTY_PATH}/openssl/lib/libcrypto.so)
set_target_properties(ssl PROPERTIES IMPORTED_LOCATION ${THIRD_PARTY_PATH}/openssl/lib/libssl.so)
set_target_properties(mysqlconn8 PROPERTIES IMPORTED_LOCATION ${THIRD_PARTY_PATH}/mysqlconn/lib/libmysqlcppconn8.so.2.8.0.33)

target_link_libraries(db PRIVATE crypto ssl mysqlconn8 common)