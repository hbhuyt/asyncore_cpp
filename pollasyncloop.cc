#include "pollasyncloop.h"

#include <sys/socket.h>	// accept
#include <poll.h>	// poll
#include <unistd.h>	// read, close

inline bool check_eagain(){
	return errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK;
}

// ===========================

PollAsyncLoop::PollAsyncLoop(uint32_t const maxClients, int const serverFD) :
				_serverFD(serverFD),
				_maxClients(maxClients),
				_statusData(new pollfd[_maxClients + 1]){
	_clearStatusData();
}

PollAsyncLoop::~PollAsyncLoop() = default;

// ===========================

void PollAsyncLoop::_clearStatusData(){
	_statusData[0].fd	= _serverFD;
	_statusData[0].events	= POLLIN;

	for(uint32_t i = 1; i <= _maxClients; ++i)
		_statusData[i].fd = -1;
}

bool PollAsyncLoop::_insertStatusData(int fd){
	for (uint32_t i = 1; i <= _maxClients; ++i)
		if (_statusData[i].fd < 0) {
			_statusData[i].fd = fd;
			_statusData[i].events = POLLIN | POLLOUT;

			++_connectedClients;

			return true;
		}

	 return false;
}

bool PollAsyncLoop::wait(int const timeout){
//	printf("we are polling...\n");

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
		if (_statusData[i].fd >= 0)
			_serviceFD(_statusData[i]);

	return true;
}

bool PollAsyncLoop::_serviceFD(pollfd &p){
	constexpr size_t buffer_size = 1024;
	static char buffer[buffer_size];

	if (p.revents & POLLERR){
		return _disconnectFD(p, true);
	}

	if (p.revents & POLLIN){
		// read
		ssize_t const size = read(p.fd, buffer, buffer_size);

		if (size < 0 && check_eagain() == false){
			// error, disconnect.
			return _disconnectFD(p, true);
		}

		if (size == 0){
			// normal, disconnect.
			return _disconnectFD(p);
		}

		printf("%5d | %5zu | [begin]%.*s[end]\n", p.fd, size, (int) size, buffer);

		return true;
	}

	return true;
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

bool PollAsyncLoop::_disconnectFD(pollfd &p, bool const err){
	::close(p.fd);

	--_connectedClients;

	if (err)
		_log("Error Disconnect", p.fd, _connectedClients);
	else
		_log("Normal Disconnect", p.fd, _connectedClients);

	p.fd = -1;

	return true;
}

