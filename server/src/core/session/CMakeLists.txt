set(SESSION_SRC
    src/chat_session.cpp
    src/user_manager.cpp
)

add_library(core_session ${SESSION_SRC})

target_include_directories(core_session PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    # ${CMAKE_CURRENT_LIST_DIR} 表示当前 CMakeLists.txt 所在目录
    ${CMAKE_CURRENT_LIST_DIR}/../include # 让 session 模块能访问 msg.h（即 src/core）
)

target_include_directories(core_session SYSTEM PRIVATE
    ${THIRD_PARTY_PATH}/jsoncpp/include    
)

add_library(jsoncpp STATIC IMPORTED)
set_target_properties(jsoncpp PROPERTIES IMPORTED_LOCATION ${THIRD_PARTY_PATH}/jsoncpp/lib/jsoncpp.a)
target_link_libraries(core_session PRIVATE jsoncpp common network util)