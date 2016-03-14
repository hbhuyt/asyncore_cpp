#ifndef _POLL_ASYNC_LOOP_H
#define _POLL_ASYNC_LOOP_H

#include <cstdint>

#include <memory>
#include <tuple>

struct pollfd;

class PollAsyncSelector{
public:
	enum class WaitStatus{ OK, NONE, ERROR };

	enum class FDStatus{ NONE, READ, WRITE, ERROR };

public:
	PollAsyncSelector(uint32_t _maxFD);
	PollAsyncSelector(PollAsyncSelector &&other) /* = default */;
	PollAsyncSelector &operator =(PollAsyncSelector &&other) /* = default */;
	~PollAsyncSelector() /* = default */;

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

#endif

