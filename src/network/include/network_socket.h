/*
 * Copyright (c) https://github.com/kaapomoi 2023.
 */

#ifndef SALMIAC_NETWORK_SOCKET_H
#define SALMIAC_NETWORK_SOCKET_H

#include <WS2tcpip.h>
#include <winsock2.h>

#include <cstdint>
#include <functional>
#include <optional>
#include <string>
#include <unordered_map>

namespace sal {

struct Packet {
    /// `address` can be target or sender address,
    /// depending on whether the use case is send or receive
    sockaddr_in address;
    std::string message;

    std::string addr_to_str() noexcept
    {
        std::string res{};
        res.resize(16);
        ::inet_ntop(AF_INET, &address.sin_addr, res.data(), res.size());
        auto first_ws_idx = res.find('\0');
        res.resize(first_ws_idx);
        return res;
    }
};


class Network_socket {
public:
    static constexpr std::uint16_t default_port{33364};
    static constexpr std::uint16_t buffer_length{256};

    Network_socket() noexcept;
    virtual ~Network_socket() noexcept;

    [[nodiscard]] bool valid() const noexcept { return m_handle != INVALID_SOCKET; }

    [[nodiscard]] std::optional<Packet> receive() const noexcept;

    virtual std::int32_t send(Packet packet) const noexcept;

protected:
    void set_non_blocking() noexcept;

    WSAData m_wsa_data{};
    SOCKET m_handle{INVALID_SOCKET};
};

} // namespace sal

#endif
