#ifndef _ASYNC_LOOP_H
#define _ASYNC_LOOP_H

#include <cstdint>
#include <map>

template<class SELECTOR>
class AsyncLoop{
public:
	constexpr static int  TIMEOUT	= 5000;

private:
	using WaitStatus	= typename SELECTOR::WaitStatus;
	using FDStatus		= typename SELECTOR::FDStatus;

private:
	class Connection;

public:
	AsyncLoop(SELECTOR &&selector, int const serverFD);
	~AsyncLoop() /* = default */;

	bool process();

private:
	void _handleRead(int fd);
	bool _handleConnect(int fd);
	void _handleDisconnect(int fd, bool error = false);

private:
	static void __log(const char *s, int const fd, uint32_t const clients){
		if (fd)
			printf("%-20s | clients: %5u | fd: %5d\n", s, clients, fd);
		else
			printf("%-20s | clients: %5u |\n",          s, clients);
	}

private:
	static bool socket__check_eagain();
	static bool socket__makeNonBlocking(int fd);

private:
	class Connection{
	public:
		constexpr static uint16_t MAX_SIZE = 16 * 1024;

	public:
		Connection(int const fd) : fd(fd){}

	public:
		int		fd;
		uint32_t	time = now();
		uint16_t	buffer_size = 0;
		char		buffer[MAX_SIZE];

	public:
		bool expired(uint32_t timeout) const{
			return time + timeout < now();
		}

	private:
		static uint32_t now();
	};

private:
	SELECTOR			_selector;
	int				_serverFD;
	std::map<int,Connection>	_connections;
	uint32_t			_connectedClients = 0;
};

// ===========================

#include "asyncloop_implementation.h"

#endif

