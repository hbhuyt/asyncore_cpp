#include "mysocket.h"

#include <sys/socket.h>		// socket
#include <arpa/inet.h>		// AF_INET
#include <fcntl.h>		// fcntl
#include <unistd.h>		// close



int MySocket::create(const char *ip, uint16_t const port, uint16_t const backlog){
	int fd = socket(AF_INET , SOCK_STREAM , 0);

	if(fd < 0)
		return -1;

	int const opt = 1;
	if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, & opt, sizeof(opt)) < 0){
		::close(fd);
		return -2;
	}

	if (! makeNonBlocking(fd)){
		::close(fd);
		return -3;
	}

	struct sockaddr_in address;

	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(port);

	if (bind(fd, (struct sockaddr *) & address, sizeof(address)) < 0){
		::close(fd);
		return -4;
	}

	if (listen(fd, backlog ? backlog : SOMAXCONN) < 0){
		::close(fd);
		return -5;
	}

	return fd;
}

void MySocket::close(int const fd) noexcept{
	if (fd >= 0)
		::close(fd);
}

int MySocket::accept(int const fd) noexcept{
	if (fd >= 0)
		return ::accept(fd, NULL, NULL);

	return -1;
}

bool MySocket::makeNonBlocking(int const fd){
	if (fcntl(fd, F_SETFL, O_NONBLOCK) < 0)
		return false;

	return true;
}

