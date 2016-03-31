#ifndef _NET_ASYNC_LOOP_H
#define _NET_ASYNC_LOOP_H

#include "selectordefs.h"
#include "connection.h"

#include <map>

namespace net{

template<class SELECTOR, class PROTOCOL, size_t CONNECTION_BUFFER_SIZE = 1024>
class AsyncLoop{
public:
	constexpr static int  WAIT_TIMEOUT	=  5;
	constexpr static int  CONN_TIMEOUT	= 20;

private:
	constexpr static int  WAIT_TIMEOUT_MS	=  WAIT_TIMEOUT * 1000;

public:
	AsyncLoop(SELECTOR &&selector, PROTOCOL &&protocol, int serverFD);
	~AsyncLoop();
	AsyncLoop(AsyncLoop &&other) = default;
	AsyncLoop &operator=(AsyncLoop &&other) = default;

	bool process();

private:
	enum class DisconnectStatus{
		NORMAL,
		ERROR,
		TIMEOUT,

		PROBLEM_MAP_NOT_FOUND,
		PROBLEM_BUFFER_READ,
		PROBLEM_BUFFER_WRITE
	};

	using ProtocolStatus		= typename PROTOCOL::Status;

	using AsyncConnection		= Connection<CONNECTION_BUFFER_SIZE>;
	using ConnectionContainer	= std::map<int, AsyncConnection>;

private:
	void handleRead_(int fd);
	void handleWrite_(int fd);
	bool handleConnect_(int fd);
	void handleDisconnect_(int fd, const DisconnectStatus error);
	bool handleProtocol_(int fd, AsyncConnection &connection);

	void handleSocketOps_(int fd, ssize_t size);

private:
	bool insertFD_(int fd);
	void removeFD_(int fd);
	void expireFD_();

private:
	void log_(const char *s, int const fd = -1) const{
		if (fd < 0)
			printf("%-20s | clients: %5u |\n",         s, connectedClients_);
		else
			printf("%-20s | clients: %5u | fd: %5d\n", s, connectedClients_, fd);
	}

private:
	SELECTOR		selector_;
	PROTOCOL		protocol_;
	int			serverFD_;
	ConnectionContainer	connections_;
	uint32_t		connectedClients_ = 0;
};


} // namespace

// ===========================

#include "asyncloop_impl.h"

#endif

