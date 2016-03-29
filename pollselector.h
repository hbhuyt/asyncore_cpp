#ifndef _NET_POLL_SELECTOR_H
#define _NET_POLL_SELECTOR_H

#include "statuses.h"

#include <cstdint>

#include <tuple>
#include <vector>

struct pollfd;

namespace net{
namespace selector{


class PollSelector{
public:
	PollSelector(uint32_t maxFD_);
	PollSelector(PollSelector &&other) /* = default */;
	PollSelector &operator =(PollSelector &&other) /* = default */;
	~PollSelector() /* = default */;

	uint32_t maxFD() const;

	bool insertFD(int fd);
	bool removeFD(int fd);

	WaitStatus wait(int timeout);

	std::tuple<int, FDStatus> getFDStatus(uint32_t no) const;

private:
	void _initializeStatusData();
	void _closeStatusData();

private:
	std::vector<pollfd>	statusData_;
};


} // namespace selector
} // namespace

#endif

