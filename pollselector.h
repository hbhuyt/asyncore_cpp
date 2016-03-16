#ifndef _NET_POLL_SELECTOR_H
#define _NET_POLL_SELECTOR_H

#include "statuses.h"

#include <cstdint>

#include <memory>
#include <tuple>

struct pollfd;

namespace net{

class PollSelector{
public:
	PollSelector(uint32_t _maxFD);
	PollSelector(PollSelector &&other) /* = default */;
	PollSelector &operator =(PollSelector &&other) /* = default */;
	~PollSelector() /* = default */;

	uint32_t maxFD() const{
		return _maxFD;
	}

	bool insertFD(int fd);
	bool removeFD(int fd);

	WaitStatus wait(int timeout);

	std::tuple<int, FDStatus> getFDStatus(uint32_t no) const;

private:
	void _initializeStatusData();
	void _closeStatusData();

private:
	uint32_t			_maxFD;
	std::unique_ptr<pollfd[]>	_statusData;
};

} // namespace

#endif

