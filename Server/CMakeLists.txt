auto_add_executable(Server)

target_link_libraries(Server PRIVATE SQLite3 ssl crypto)