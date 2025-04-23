set(SESSION_SRC
    src/chat_session.cpp
)

add_library(core_session ${SESSION_SRC})

target_include_directories(core_session PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(core_session PRIVATE network util)