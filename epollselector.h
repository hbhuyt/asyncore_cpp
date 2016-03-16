#ifndef _NET_EPOLL_SELECTOR_H
#define _NET_EPOLL_SELECTOR_H

#include "statuses.h"

#include <cstdint>

#include <memory>
#include <tuple>

struct epoll_event;

namespace net{

class EPollSelector{
public:
	EPollSelector(uint32_t _maxFD);
	EPollSelector(EPollSelector &&other) /* = default */;
	EPollSelector &operator =(EPollSelector &&other) /* = default */;
	~EPollSelector() /* = default */;

	void swap(EPollSelector &other);

	uint32_t maxFD() const{
		return _maxFD;
	}

	bool insertFD(int fd);
	bool removeFD(int fd);

	WaitStatus wait(int timeout);

	std::tuple<int, FDStatus> getFDStatus(uint32_t no) const;

private:
	void _initializeEPoll();
	void _closeEPoll();

private:
	int				_epollFD;
	uint32_t			_maxFD;
	std::unique_ptr<epoll_event[]>	_statusData;
	int				_statusCount	= 0;
};



} // namespace

#endif

