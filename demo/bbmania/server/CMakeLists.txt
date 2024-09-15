add_library(bbmania_server STATIC
        src/bbmania_server.cpp
)

target_include_directories(bbmania_server PUBLIC include ../include)
target_link_libraries(bbmania_server PUBLIC core common network)
