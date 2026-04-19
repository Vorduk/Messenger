add_library(SQLite3 STATIC sqlite3.c)

target_include_directories(SQLite3 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

set_source_files_properties(sqlite3.c PROPERTIES LANGUAGE C)
