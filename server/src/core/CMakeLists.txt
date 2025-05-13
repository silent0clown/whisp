# add_subdirectory(audio)
# add_subdirectory(concurrency)
# add_subdirectory(message)
# add_subdirectory(login)  # 如果未来要启用，再打开

# The 'core' library is defined as an INTERFACE because it is used to propagate
# include directories and link dependencies to other targets without producing
# any compiled output.


file(GLOB_RECURSE CORE_SRC CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp    
    ${CMAKE_CURRENT_SOURCE_DIR}/src/session*.cpp    
)
add_library(core ${CORE_SRC})

target_include_directories(core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include/session
    ${THIRD_PARTY_PATH}/jsoncpp/include    
)

add_library(jsoncpp STATIC IMPORTED)
set_target_properties(jsoncpp PROPERTIES IMPORTED_LOCATION ${THIRD_PARTY_PATH}/jsoncpp/lib/libjsoncpp.a)
# add_subdirectory(session)

target_link_libraries(core PUBLIC
    # core_session
    common
    db 
    network 
    util
    jsoncpp
)