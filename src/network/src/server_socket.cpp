/*
 * Copyright (c) https://github.com/kaapomoi 2023.
 */

#include "server_socket.h"

#include "log.h"

#include <winsock2.h>

namespace sal {


Server_socket::Server_socket(std::string const& server_bind_addr) noexcept : Network_socket()
{
    // Create a server socket
    m_handle = ::socket(AF_INET, SOCK_DGRAM, 0);
    if (m_handle == INVALID_SOCKET) {
        // Log or throw an exception
        Log::error("::socket failed for server socket: {} \n", WSAGetLastError());
    }

    sockaddr_in server{};
    server.sin_family = AF_INET;
    ::inet_pton(AF_INET, server_bind_addr.c_str(), &server.sin_addr.S_un.S_addr);
    server.sin_port = ::htons(default_port);

    // Bind the socket
    if (::bind(m_handle, (sockaddr*)&server, sizeof(server)) == SOCKET_ERROR) {
        Log::error("::bind failed for server socket: {} \n", WSAGetLastError());
        ::closesocket(m_handle);
        m_handle = INVALID_SOCKET;
        ::WSACleanup();
    }

    set_non_blocking();
}


} // namespace sal