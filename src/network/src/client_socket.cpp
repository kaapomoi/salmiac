/*
 * Copyright (c) https://github.com/kaapomoi 2023.
 */

#include "client_socket.h"

#include "log.h"

namespace sal {

Client_socket::Client_socket(std::string const& target_server_address) noexcept : Network_socket()
{
    m_handle = ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (m_handle == SOCKET_ERROR) {
        Log::error("::socket() failed for socket: {} \n", WSAGetLastError());
    }

    // Resolve the server address and port
    ::memset((char*)&m_server_addr, 0, sizeof(m_server_addr));
    m_server_addr.sin_family = AF_INET;
    m_server_addr.sin_port = ::htons(default_port);
    ::inet_pton(AF_INET, target_server_address.c_str(), &m_server_addr.sin_addr.S_un.S_addr);

    set_non_blocking();
}


std::int32_t Client_socket::send(Packet packet) const noexcept
{
    return ::sendto(m_handle, packet.message.data(),
                    static_cast<std::int32_t>(packet.message.size()), 0, (sockaddr*)&m_server_addr,
                    sizeof(sockaddr_in));
}

} // namespace sal