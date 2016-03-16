#include "epollselector.h"

#include <sys/epoll.h>	// epoll
#include <unistd.h>	// close, for _closeEPoll()

namespace net{



EPollSelector::EPollSelector(uint32_t const maxFD) :
				_maxFD(maxFD),
				_statusData(new epoll_event[maxFD]){
	_initializeEPoll();
}

EPollSelector::EPollSelector(EPollSelector &&other) :
				_epollFD	(std::move(other._epollFD	)),
				_maxFD		(std::move(other._maxFD		)),
				_statusData	(std::move(other._statusData	)),
				_statusCount	(std::move(other._statusCount	)){
	other._epollFD = -1;
}


EPollSelector &EPollSelector::operator =(EPollSelector &&other){
	swap(other);

	return *this;
}

void EPollSelector::swap(EPollSelector &other){
	using std::swap;

	swap(_epollFD		, other._epollFD	);
	swap(_maxFD		, other._maxFD		);
	swap(_statusData	, other._statusData	);
	swap(_statusCount	, other._statusCount	);
}


EPollSelector::~EPollSelector(){
	if (_statusData)
		_closeEPoll();
}

// ===========================

WaitStatus EPollSelector::wait(int const timeout){
	if (_epollFD < 0)
		return WaitStatus::ERROR;

	_statusCount = epoll_wait(_epollFD, _statusData.get(), _maxFD, timeout);

	if (_statusCount < 0)
		return WaitStatus::ERROR;

	if (_statusCount == 0)
		return WaitStatus::NONE;

	return WaitStatus::OK;
}

std::tuple<int, FDStatus> EPollSelector::getFDStatus(uint32_t const no) const{
	if (no < (uint32_t) _statusCount){
		const epoll_event &ev = _statusData[no];

		int const fd = ev.data.fd;

		if (ev.events & EPOLLERR)
			return std::make_tuple(fd, FDStatus::ERROR);

		if ((ev.events & EPOLLIN) || (ev.events & EPOLLHUP))
			return std::make_tuple(fd, FDStatus::READ);

		if (ev.events & EPOLLOUT)
			return std::make_tuple(fd, FDStatus::WRITE);
	}

	return std::make_tuple(-1, FDStatus::STOP);
}

bool EPollSelector::insertFD(int const fd){
	epoll_event ev;
	ev.events = EPOLLIN;
	ev.data.fd = fd;

	int const result = epoll_ctl(_epollFD, EPOLL_CTL_ADD, fd, &ev);

	return result >= 0;
}

bool EPollSelector::removeFD(int const fd){
	int const result = epoll_ctl(_epollFD, EPOLL_CTL_DEL, fd, nullptr);

	return result >= 0;
}

// ===========================

void EPollSelector::_initializeEPoll(){
	_epollFD = epoll_create(_maxFD);
}

void EPollSelector::_closeEPoll(){
	::close(_epollFD);
}



} // namespace

