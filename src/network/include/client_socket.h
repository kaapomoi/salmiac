/*
 * Copyright (c) https://github.com/kaapomoi 2023.
 */

#ifndef SALMIAC_CLIENT_SOCKET_H
#define SALMIAC_CLIENT_SOCKET_H

#include "network_socket.h"

namespace sal {

class Client_socket : public Network_socket {
public:
    explicit Client_socket(std::string const& target_server_address) noexcept;

    std::int32_t send(Packet packet) const noexcept final;

private:
    sockaddr_in m_server_addr{};
};

} // namespace sal

#endif //SALMIAC_CLIENT_SOCKET_H
