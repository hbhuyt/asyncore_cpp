#ifndef _NET_SOCKETS_H
#define _NET_SOCKETS_H

#include <cstdint>

namespace net{

int socket_create(const char *ip, uint16_t port, uint16_t backlog = 0, bool nonblock = true) noexcept;

bool socket_makeNonBlocking(int fd) noexcept;

int socket_accept(int fd) noexcept;

void socket_close(int fd) noexcept;

bool socket_check_eagain() noexcept;

}

#endif

