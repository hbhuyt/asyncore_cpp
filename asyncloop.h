#ifndef _NET_ASYNC_LOOP_H
#define _NET_ASYNC_LOOP_H

#include "statuses.h"
#include "connection.h"

#include <map>

namespace net{

template<class SELECTOR, class CONNECTION = Connection<> >
class AsyncLoop{
public:
	constexpr static int  WAIT_TIMEOUT	=  5;
	constexpr static int  CONN_TIMEOUT	= 20;

private:
	constexpr static int  WAIT_TIMEOUT_MS	=  WAIT_TIMEOUT * 1000;

public:
	AsyncLoop(SELECTOR &&selector, int serverFD);
	~AsyncLoop();
	AsyncLoop(AsyncLoop &&other) = default;
	AsyncLoop &operator=(AsyncLoop &&other) = default;

	bool process();

private:
	enum class DisconnecStatus{ NORMAL, ERROR, PROBLEM, TIMEOUT };

private:
	void _handleRead(int fd);
	bool _handleConnect(int fd);
	void _handleDisconnect(int fd, const DisconnecStatus &error);

private:
	bool _insertFD(int fd);
	void _removeFD(int fd);
	void _expireFD();

private:
	void __log(const char *s, int const fd = -1) const{
		if (fd < 0)
			printf("%-20s | clients: %5u |\n",         s, _connectedClients);
		else
			printf("%-20s | clients: %5u | fd: %5d\n", s, _connectedClients, fd);
	}

private:
	SELECTOR			_selector;
	int				_serverFD;
	std::map<int,CONNECTION>	_connections;
	uint32_t			_connectedClients = 0;
};


} // namespace

// ===========================

#include "asyncloop_implementation.h"

#endif

