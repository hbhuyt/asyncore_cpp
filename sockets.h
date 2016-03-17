#ifndef _NET_SOCKETS_H
#define _NET_SOCKETS_H

#include <cstdint>

namespace net{

constexpr int SOCKET_NONBLOCK	= 0x01;
constexpr int SOCKET_TCPNODELAY	= 0x02;

// ===========================

constexpr int SOCKET_DEFAULTOPT	= SOCKET_NONBLOCK & SOCKET_TCPNODELAY;

// ===========================

int socket_create(const char *ip, uint16_t port, uint16_t backlog = 0, int options = SOCKET_DEFAULTOPT) noexcept;

bool socket_makeNonBlocking(int fd) noexcept;
bool socket_makeTCPNoDelay(int fd) noexcept;

int socket_accept(int fd) noexcept;

void socket_close(int fd) noexcept;

bool socket_check_eagain() noexcept;

}

#endif

