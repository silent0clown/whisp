# add_subdirectory(audio)
# add_subdirectory(concurrency)
add_subdirectory(session)
# add_subdirectory(message)
# add_subdirectory(login)  # 如果未来要启用，再打开

# The 'core' library is defined as an INTERFACE because it is used to propagate
# include directories and link dependencies to other targets without producing
# any compiled output.
add_library(core INTERFACE)

target_include_directories(core INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/
)

target_link_libraries(core INTERFACE
    core_session
)