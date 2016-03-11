#ifndef _MY_SOCKET_H
#define _MY_SOCKET_H

#include <cstdint>

namespace MySocket{
	int create(const char *ip, uint16_t port, uint16_t backlog = 0);

	void close(int fd) noexcept;

	int accept(int fd) noexcept;

	bool makeNonBlocking(int fd);
};

#endif

