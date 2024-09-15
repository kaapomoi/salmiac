/*
 * Copyright (c) https://github.com/kaapomoi 2024.
 */

#ifndef BBMANIA_SERVER_CLIENT_CONNECTION_H
#define BBMANIA_SERVER_CLIENT_CONNECTION_H

#include "network_socket.h"

struct Client_connection {
    sockaddr_in address;
    std::string unique_client_id;
    bool ready_to_start_game{false};
};


#endif //BBMANIA_SERVER_CLIENT_CONNECTION_H
