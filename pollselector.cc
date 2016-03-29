#include "pollselector.h"

#include <poll.h>	// poll
#include <unistd.h>	// close, for closeStatusData_()

namespace net{
namespace selector{


PollSelector::PollSelector(uint32_t const maxFD) :
				statusData_(maxFD){
	initializeStatusData_();
}

PollSelector::PollSelector(PollSelector &&other) = default;

PollSelector &PollSelector::operator =(PollSelector &&other) = default;

PollSelector::~PollSelector(){
	closeStatusData_();
}

// ===========================

uint32_t PollSelector::maxFD() const{
	return (uint32_t) statusData_.size();
}

WaitStatus PollSelector::wait(int const timeout){
	int const activity = poll(statusData_.data(), statusData_.size(), timeout);

	if (activity < 0)
		return WaitStatus::ERROR;

	if (activity == 0)
		return WaitStatus::NONE;

	return WaitStatus::OK;
}

std::tuple<int, FDStatus> PollSelector::getFDStatus(uint32_t const no) const{
	const auto &p = statusData_[no];
	int  const fd = p.fd;
	auto const ev = p.revents;

	if (ev & POLLERR)
		return std::make_tuple(fd, FDStatus::ERROR);

	if (ev & POLLIN)
		return std::make_tuple(fd, FDStatus::READ);

	if (ev & POLLOUT)
		return std::make_tuple(fd, FDStatus::WRITE);

	return std::make_tuple(fd, FDStatus::NONE);
}

bool PollSelector::insertFD(int const fd){
	uint32_t pos = 0;
	bool     pok = false;

	for(uint32_t i = 0; i < statusData_.size(); ++i){
		if (statusData_[i].fd == fd){
			pos = i;
			pok = true;
			break;
		}

		if (pok == false && statusData_[i].fd < 0){
			pos = i;
			pok = true;
		}
	}

	if (! pok)
		return false;

	statusData_[pos].fd = fd;
	statusData_[pos].events = POLLIN;// | POLLOUT;

	return true;
}

bool PollSelector::removeFD(int const fd){
	// bit ugly.
	for(auto &item : statusData_)
		if (item.fd == fd){
			item.fd = -1;
			return true;
		}

	return false;
}

// ===========================

void PollSelector::initializeStatusData_(){
	for(auto &item : statusData_)
		item.fd = -1;
}

void PollSelector::closeStatusData_(){
	for(auto &item : statusData_)
		if (item.fd >= 0)
			::close(item.fd);
}


} // namespace selector
} // namespace

