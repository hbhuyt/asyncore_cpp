#include "pollasyncloop.h"

#include <sys/socket.h>	// accept
#include <poll.h>	// poll
#include <unistd.h>	// read, close

#define BLOCKING_CLIENT_SOCKETS

inline bool check_eagain(){
	return errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK;
}

// ===========================

PollAsyncLoop::PollAsyncLoop(uint32_t const maxClients, int const serverFD) :
				_serverFD(serverFD),
				_maxClients(maxClients),
				_statusData(new pollfd[_maxClients + 1]){
	_initializeStatusData();
}

PollAsyncLoop::~PollAsyncLoop() = default;

// ===========================

void PollAsyncLoop::_initializeStatusData(){
	_statusData[0].fd	= _serverFD;
	_statusData[0].events	= POLLIN;

	for(uint32_t i = 1; i <= _maxClients; ++i)
		_statusData[i].fd = -1;
}

bool PollAsyncLoop::_insertStatusData(int fd){
	uint32_t here = 0;
	for (uint32_t i = 1; i <= _maxClients; ++i){
		if (_statusData[i].fd == fd){
			here = i;
			break;
		}

		if (here == 0 && _statusData[i].fd < 0)
			here = i;
	}

	if (here == 0)
		return false;

	_statusData[here].fd = fd;
	_statusData[here].events = POLLIN;// | POLLOUT;

	++_connectedClients;

	return true;
}

void PollAsyncLoop::_removeStatusData(pollfd &p, const Status &stat){
	--_connectedClients;

	if (stat == Status::ERROR)
		_log("Error Disconnect", p.fd, _connectedClients);
	else
		_log("Normal Disconnect", p.fd, _connectedClients);

	_disconnectFD(p.fd);

	p.fd = -1;
}

bool PollAsyncLoop::wait(int const timeout){
	_log("poll()-ing...", 0, _connectedClients);

	int const activity = poll(_statusData.get(), _maxClients + 1, timeout);

	if (activity < 0){
		_log("poll() error", 0, _connectedClients);

		return false;
	}

	if (activity == 0)
		return true;

	// check and accept incomming connections
	if (_statusData[0].revents & POLLIN){
		_connectFD();
		// always return true here.
		return true;
	}

	for(uint32_t i = 1; i <= _maxClients; ++i)
		if (_statusData[i].fd >= 0){
			Status result = _serviceFD(_statusData[i]);

			if (result == Status::ERROR || result == Status::DISCONNECT)
				_removeStatusData(_statusData[i], result);
		}

	return true;
}

auto PollAsyncLoop::_serviceFD(pollfd &p) -> Status{
	if (p.revents & POLLERR){
		return Status::ERROR;
	}

	if (p.revents & POLLIN){
		return _readFD(p.fd);
	}

	return Status::DONE;
}

auto PollAsyncLoop::_readFD(int const fd) -> Status{
	#ifdef BLOCKING_CLIENT_SOCKETS
		return _readFDSingle(fd);
	#else

	for(;;){
		Status result = _readFDSingle(fd);

		if (result == Status::ERROR || result == Status::DONE)
			return result;
	}

	#endif
}

auto PollAsyncLoop::_readFDSingle(int const fd) -> Status{
	constexpr size_t buffer_size = 1024;
	static char buffer[buffer_size];

	ssize_t const size = read(fd, buffer, buffer_size);

	if (size < 0){
		if ( check_eagain() ){
			// no more data to read...
			return Status::DONE;
		}

		// error, disconnect.
		return Status::ERROR;
	}

	if (size == 0){
		// normal, disconnect.
		return Status::DISCONNECT;
	}

	printf("%5d | %5zu | [begin]%.*s[end]\n", fd, size, (int) size, buffer);

	return Status::OK;
}

bool PollAsyncLoop::_connectFD(){
	while(_connectFDSingle());

	return true;
}

bool PollAsyncLoop::_connectFDSingle(){
	int const newFD = accept( _serverFD, nullptr, nullptr);

	// _serverFD is non blocking, so we do not need to check EAGAIN
	if (newFD < 0)
		return false;

	// put it into the array
	if ( _insertStatusData(newFD) ){
		_log("Connect", newFD, _connectedClients);

		return true;
	}else{
		_log("Drop connect", newFD, _connectedClients);

		::close(newFD);

		return true;
	}
}

void PollAsyncLoop::_disconnectFD(int const fd){
	::close(fd);
}

