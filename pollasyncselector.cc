#include "pollasyncselector.h"

#include <poll.h>	// poll
#include <unistd.h>	// close, for _closeStatusData()

PollAsyncSelector::PollAsyncSelector(uint32_t const maxFD) :
				_maxFD(maxFD),
				_statusData(new pollfd[_maxFD]){
	_initializeStatusData();
}

PollAsyncSelector::PollAsyncSelector(PollAsyncSelector &&other) = default;

PollAsyncSelector &PollAsyncSelector::operator =(PollAsyncSelector &&other) = default;

PollAsyncSelector::~PollAsyncSelector(){
	if (_statusData)
		_closeStatusData();
}

// ===========================

auto PollAsyncSelector::wait(int const timeout) -> WaitStatus{
	int const activity = poll(_statusData.get(), _maxFD, timeout);

	if (activity < 0)
		return WaitStatus::ERROR;

	if (activity == 0)
		return WaitStatus::NONE;

	return WaitStatus::OK;
}

auto PollAsyncSelector::getFDStatus(uint32_t const no) const -> std::tuple<int, FDStatus>{
	const auto &p = _statusData[no];
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

bool PollAsyncSelector::insertFD(int const fd){
	uint32_t pos = 0;
	bool     pok = false;

	for(uint32_t i = 0; i < _maxFD; ++i){
		if (_statusData[i].fd == fd){
			pos = i;
			pok = true;
			break;
		}

		if (pok == false && _statusData[i].fd < 0){
			pos = i;
			pok = true;
		}
	}

	if (! pok)
		return false;

	_statusData[pos].fd = fd;
	_statusData[pos].events = POLLIN;// | POLLOUT;

	return true;
}

bool PollAsyncSelector::removeFD(int const fd){
	// bit ugly.
	for(uint32_t i = 0; i < _maxFD; ++i)
		if (_statusData[i].fd == fd){
			_statusData[i].fd = -1;
			return true;
		}

	return false;
}

// ===========================

void PollAsyncSelector::_initializeStatusData(){
	for(uint32_t i = 0; i < _maxFD; ++i)
		_statusData[i].fd = -1;
}

void PollAsyncSelector::_closeStatusData(){
	for(uint32_t i = 0; i < _maxFD; ++i)
		if (_statusData[i].fd >= 0)
			::close(_statusData[i].fd);
}

