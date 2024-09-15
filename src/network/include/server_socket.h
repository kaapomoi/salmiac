/*
 * Copyright (c) https://github.com/kaapomoi 2023.
 */

#ifndef SALMIAC_SERVER_SOCKET_H
#define SALMIAC_SERVER_SOCKET_H

#include "network_socket.h"

namespace sal {

class Server_socket : public Network_socket {
public:
    explicit Server_socket(std::string const& server_bind_addr) noexcept;
};

} // namespace sal


#endif //SALMIAC_SERVER_SOCKET_H
