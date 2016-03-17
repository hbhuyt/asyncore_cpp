#include "sockets.h"

#include <sys/socket.h>		// EAGAIN
#include <arpa/inet.h>		// AF_INET
#include <netinet/tcp.h>	// TCP_NODELAY
#include <fcntl.h>		// fcntl

#include <unistd.h>		// close

#include <errno.h>		// errno

namespace net{



bool socket_check_eagain() noexcept{
	return errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK;
}

bool socket_makeNonBlocking(int const fd) noexcept{
	if (fd >= 0)
		return fcntl(fd, F_SETFL, O_NONBLOCK) >= 0;

	return false;
}

bool socket_makeTCPNoDelay(int const fd) noexcept{
	int const opt = 1;
	return (setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, & opt, sizeof opt) >= 0);
}

void socket_close(int const fd) noexcept{
	if (fd >= 0)
		::close(fd);
}

int socket_accept(int const fd) noexcept{
	if (fd >= 0)
		return ::accept(fd, NULL, NULL);

	return -1;
}

int socket_create(const char *ip, uint16_t const port, uint16_t const backlog, int const options) noexcept{
	int fd = socket(AF_INET , SOCK_STREAM , 0);

	if(fd < 0)
		return -1;

	int const opt = 1;
	if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, & opt, sizeof opt) < 0){
		::close(fd);
		return -2;
	}

	if (options & SOCKET_NONBLOCK)
	if (! socket_makeNonBlocking(fd) ){
		::close(fd);
		return -3;
	}

	if (options & SOCKET_TCPNODELAY)
	if (! socket_makeTCPNoDelay(fd) ){
		::close(fd);
		return -4;
	}

	struct sockaddr_in address;

	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(port);

	if (bind(fd, (struct sockaddr *) & address, sizeof address) < 0){
		::close(fd);
		return -5;
	}

	if (listen(fd, backlog ? backlog : SOMAXCONN) < 0){
		::close(fd);
		return -6;
	}

	return fd;
}



} // namespace

