#ifndef _NET_EPOLL_SELECTOR_H
#define _NET_EPOLL_SELECTOR_H

#include "selectordefs.h"

#include <cstdint>

#include <vector>

struct epoll_event;

namespace net{
namespace selector{


class EPollSelector{
public:
	EPollSelector(uint32_t maxFD_);
	EPollSelector(EPollSelector &&other) /* = default */;
	EPollSelector &operator =(EPollSelector &&other) /* = default */;
	~EPollSelector() /* = default */;

	void swap(EPollSelector &other);

	uint32_t maxFD() const;

	bool insertFD(int fd);
	bool removeFD(int fd);

	WaitStatus wait(int timeout);

	FDResult getFDStatus(uint32_t no) const;

private:
	void _initializeEPoll();
	void _closeEPoll();

private:
	int				epollFD_;
	std::vector<epoll_event>	statusData_;
	int				statusCount_	= 0;
};


} // namespace selector
} // namespace

#endif

