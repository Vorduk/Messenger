auto_add_executable(Client)
target_compile_definitions(Client PRIVATE GLEW_STATIC)
target_link_libraries(Client PRIVATE SQLite3 ssl crypto)
