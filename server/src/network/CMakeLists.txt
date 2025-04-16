# server/src/network/CmakeLists.txt
# file(GLOB_RECURSE NETWORK_SRC CONFIGURE_DEPENDS
#     ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
# )

set(NETWORK_SRC
    src/reactor.cpp
    src/inet_address.cpp
    src/sockets.cpp
)

add_library(network ${NETWORK_SRC})

target_include_directories(network PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)



# target_include_directories(network SYSTEM PUBLIC
#     ${THIRD_PARTY_PATH}/openssl/include
#     ${THIRD_PARTY_PATH}/mysqlconn/include
# )



# add_library(crypto SHARED IMPORTED)
# add_library(ssl SHARED IMPORTED)
# add_library(mysqlconn8 SHARED IMPORTED)
# set_target_properties(crypto PROPERTIES IMPORTED_LOCATION ${THIRD_PARTY_PATH}/openssl/lib/libcrypto.so)
# set_target_properties(ssl PROPERTIES IMPORTED_LOCATION ${THIRD_PARTY_PATH}/openssl/lib/libssl.so)
# set_target_properties(mysqlconn8 PROPERTIES IMPORTED_LOCATION ${THIRD_PARTY_PATH}/mysqlconn/lib/libmysqlcppconn8.so.2.8.0.33)

target_link_libraries(network PRIVATE common util)