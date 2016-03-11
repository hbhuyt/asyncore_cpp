#ifndef _POLL_ASYNC_LOOP_H
#define _POLL_ASYNC_LOOP_H

#include <cstdint>

#include <memory>
#include <tuple>

#include <cstdio>

struct pollfd;

class PollAsyncLoop{
public:
	constexpr static int TIMEOUT = 1000;

public:
	PollAsyncLoop(uint32_t maxClients, int serverFD);
	~PollAsyncLoop() /* = default */;

	bool wait(int timeout = TIMEOUT);

private:
	static void _log(const char *s, int const fd, uint32_t const clients){
		printf("%-20s fd: %5d, conn clients: %5u\n", s, fd, clients);
	}

private:
	void _clearStatusData();
	bool _insertStatusData(int fd);
	bool _serviceFD(pollfd &p);
	bool _disconnectFD(pollfd &p, bool err = false);
	bool _connectFD();
	bool _connectFDSingle();

private:
	int				_serverFD;
	uint32_t			_maxClients;
	uint32_t			_connectedClients = 0;
	std::unique_ptr<pollfd[]> 	_statusData;
};

#endif

