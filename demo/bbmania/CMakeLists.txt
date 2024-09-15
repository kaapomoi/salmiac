add_subdirectory(server)

add_library(bbmania STATIC
        src/bbmania.cpp
        src/board.cpp
        src/network_protocol.cpp
)

target_include_directories(bbmania PUBLIC include)
target_link_libraries(bbmania PUBLIC core common network)
