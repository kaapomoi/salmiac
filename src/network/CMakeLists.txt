add_library(network STATIC
        src/network_socket.cpp
        src/server_socket.cpp
        src/client_socket.cpp
)

target_include_directories(network PUBLIC include)
target_link_libraries(network PUBLIC wsock32 ws2_32 util)
