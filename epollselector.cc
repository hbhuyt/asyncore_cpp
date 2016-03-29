#include "epollselector.h"

#include <sys/epoll.h>	// epoll
#include <unistd.h>	// close, for _closeEPoll()

namespace net{
namespace selector{


EPollSelector::EPollSelector(uint32_t const maxFD) :
				statusData_(maxFD){
	_initializeEPoll();
}

EPollSelector::EPollSelector(EPollSelector &&other) :
				epollFD_	(std::move(other.epollFD_	)),
				statusData_	(std::move(other.statusData_	)),
				statusCount_	(std::move(other.statusCount_	)){
	other.epollFD_ = -1;
}


EPollSelector &EPollSelector::operator =(EPollSelector &&other){
	swap(other);

	return *this;
}

void EPollSelector::swap(EPollSelector &other){
	using std::swap;

	swap(epollFD_		, other.epollFD_	);
	swap(statusData_	, other.statusData_	);
	swap(statusCount_	, other.statusCount_	);
}


EPollSelector::~EPollSelector(){
	_closeEPoll();
}

// ===========================

uint32_t EPollSelector::maxFD() const{
	return (uint32_t) statusData_.size();
}

WaitStatus EPollSelector::wait(int const timeout){
	if (epollFD_ < 0)
		return WaitStatus::ERROR;

	statusCount_ = epoll_wait(epollFD_, statusData_.data(), (int) statusData_.size(), timeout);

	if (statusCount_ < 0)
		return WaitStatus::ERROR;

	if (statusCount_ == 0)
		return WaitStatus::NONE;

	return WaitStatus::OK;
}

std::tuple<int, FDStatus> EPollSelector::getFDStatus(uint32_t const no) const{
	if (no < (uint32_t) statusCount_){
		const epoll_event &ev = statusData_[no];

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

	int const result = epoll_ctl(epollFD_, EPOLL_CTL_ADD, fd, &ev);

	return result >= 0;
}

bool EPollSelector::removeFD(int const fd){
	int const result = epoll_ctl(epollFD_, EPOLL_CTL_DEL, fd, nullptr);

	return result >= 0;
}

// ===========================

void EPollSelector::_initializeEPoll(){
	epollFD_ = epoll_create((int) statusData_.size());
}

void EPollSelector::_closeEPoll(){
	::close(epollFD_);
}


} // namespace selector
} // namespace

