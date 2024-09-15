/*
 * Copyright (c) https://github.com/kaapomoi 2023.
 */

#include "network_socket.h"

#include "log.h"

namespace sal {

Network_socket::Network_socket() noexcept
{
    std::int32_t rc{0};

    rc = ::WSAStartup(MAKEWORD(2, 2), &m_wsa_data);
    if (rc != 0) {
        sal::Log::error("Could not initialize network socket!");
        return;
    }
}

Network_socket::~Network_socket() noexcept
{
    closesocket(m_handle);
    m_handle = INVALID_SOCKET;
    ::WSACleanup();
}

void Network_socket::set_non_blocking() noexcept
{
    if (valid()) {
        DWORD non_block{1};
        if (ioctlsocket(m_handle, FIONBIO, &non_block) != 0) {
            sal::Log::error("Could not set socket {} to nonblocking!", m_handle);
        }
        return;
    }
    else {
        sal::Log::error("Socket is non-valid. Won't set to nonblocking!");
    }
}

std::optional<Packet> Network_socket::receive() const noexcept
{
    Packet packet;

    std::string buffer;
    buffer.resize(buffer_length);

    std::int32_t sockaddrin_len{sizeof(sockaddr_in)};
    std::int32_t message_len{::recvfrom(m_handle, buffer.data(), buffer_length, 0,
                                        (sockaddr*)&packet.address, &sockaddrin_len)};
    if (message_len == SOCKET_ERROR) {
        return {};
    }

    packet.message.swap(buffer);

    packet.message.resize(message_len);

    return packet;
}


std::int32_t Network_socket::send(Packet packet) const noexcept
{
    return ::sendto(m_handle, packet.message.data(),
                    static_cast<std::int32_t>(packet.message.size()), 0, (sockaddr*)&packet.address,
                    sizeof(sockaddr_in));
}


} // namespace sal
