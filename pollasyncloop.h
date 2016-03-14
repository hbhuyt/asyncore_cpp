#ifndef _POLL_ASYNC_LOOP_H
#define _POLL_ASYNC_LOOP_H

#include <cstdint>

#include <memory>
#include <tuple>

#include <cstdio>

struct pollfd;

class PollAsyncLoop{
public:
	constexpr static int  TIMEOUT	= 5000;

public:
	PollAsyncLoop(uint32_t maxClients, int serverFD);
	~PollAsyncLoop() /* = default */;

	bool wait(int timeout = TIMEOUT);

private:
	static void _log(const char *s, int const fd, uint32_t const clients){
		if (fd)
			printf("%-20s, clients: %5u, fd: %5d\n", s, clients, fd);
		else
			printf("%-20s, clients: %5u\n",          s, clients);
	}

private:
	enum class Status{ OK, DONE, ERROR, DISCONNECT };

private:
	void _initializeStatusData();

	bool _insertStatusData(int fd);
	void _removeStatusData(pollfd &p, const Status &stat);

	Status _serviceFD(pollfd &p);

	bool _connectFD();
	bool _connectFDSingle();

	void _disconnectFD(int fd);

	Status _readFD(int fd);
	Status _readFDSingle(int fd);

private:
	int				_serverFD;
	uint32_t			_maxClients;
	uint32_t			_connectedClients = 0;
	std::unique_ptr<pollfd[]> 	_statusData;
};

#endif

