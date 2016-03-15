#ifndef _ASYNC_LOOP_H
#define _ASYNC_LOOP_H

#include <cstdint>
#include <map>

#include "async.h"

namespace Async{

template<class SELECTOR, class CONNECTION = Connection<> >
class Loop{
public:
	constexpr static int  WAIT_TIMEOUT	=  5;
	constexpr static int  CONN_TIMEOUT	= 20;

private:
	constexpr static int  WAIT_TIMEOUT_MS	=  WAIT_TIMEOUT * 1000;

public:
	Loop(SELECTOR &&selector, int serverFD);
	~Loop() = default;

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
	SELECTOR			_selector;
	int				_serverFD;
	std::map<int,CONNECTION>	_connections;
	uint32_t			_connectedClients = 0;
};


}; // namespace


// ===========================

#include "asyncloop_implementation.h"

#endif

