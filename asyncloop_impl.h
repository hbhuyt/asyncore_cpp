#include "sockets.h"

#include <unistd.h>	// read

namespace net{

template<class SELECTOR, class CONNECTION>
AsyncLoop<SELECTOR, CONNECTION>::AsyncLoop(SELECTOR &&selector, int const serverFD) :
					_selector(std::move(selector)),
					_serverFD(serverFD){
	socket_makeNonBlocking(_serverFD);
	_selector.insertFD(_serverFD);
}

template<class SELECTOR, class CONNECTION>
AsyncLoop<SELECTOR, CONNECTION>::~AsyncLoop(){
	// _serverFD will be closed if we close epoll
	_selector.removeFD(_serverFD);
}

// ===========================

template<class SELECTOR, class CONNECTION>
bool AsyncLoop<SELECTOR, CONNECTION>::process(){
	log_("poll()-ing...");
	const WaitStatus &status = _selector.wait(WAIT_TIMEOUT_MS);

	if (status == WaitStatus::ERROR){
		log_("poll() error");

		return false;
	}

	if (status == WaitStatus::NONE){
		// idle loop, check for expired conn
		_expireFD();
		return true;
	}

	for(uint32_t i = 0; i < _selector.maxFD(); ++i){
		const auto &t = _selector.getFDStatus(i);

		switch(std::get<1>(t)){
		case FDStatus::READ:
			handleRead_( std::get<0>(t) );
			break;

		case FDStatus::ERROR:
			handleDisconnect_( std::get<0>(t), DisconnecStatus::ERROR );
			break;

		case FDStatus::STOP:
			goto break2;

		case FDStatus::NONE:
		default:
			break;

		}
	}

	break2: // label for goto... ;)

	return true;
}

// ===========================

template<class SELECTOR, class CONNECTION>
void AsyncLoop<SELECTOR, CONNECTION>::handleRead_(int const fd){
	if (fd == _serverFD){
		while (handleConnect_(fd));
		return;
	}

	// -------------------------------------

	auto it = _connections.find(fd);

	if (it == _connections.end())
		return handleDisconnect_(fd, DisconnecStatus::PROBLEM);

	auto &c = it->second;

	// -------------------------------------

	ssize_t const sizeRead = c.buffer_max - c.buffer_size;

	if (sizeRead <= 0){
		// buffer will overfow, disconnect.
		return handleDisconnect_(fd, DisconnecStatus::PROBLEM);
	}

	ssize_t const size = read(fd, & c.buffer[c.buffer_size], sizeRead);

	// -------------------------------------

	if (size < 0){
		if ( socket_check_eagain() ){
			// try again
			return;
		}else{
			// error, disconnect.
			return handleDisconnect_(fd, DisconnecStatus::ERROR);
		}
	}

	if (size == 0){
		// normal, disconnect.
		return handleDisconnect_(fd, DisconnecStatus::NORMAL);
	}

	c.buffer_size = (uint16_t) (c.buffer_size + size);

	c.refresh();

	printf("%5d | %5u | [begin]%.*s[end]\n", fd, c.buffer_size, (int) c.buffer_size, c.buffer);
}

template<class SELECTOR, class CONNECTION>
bool AsyncLoop<SELECTOR, CONNECTION>::handleConnect_(int const fd){
	// fd is same as _serverFD
	int const newFD = socket_accept(fd);

	// _serverFD is non blocking, so we do not need to check EAGAIN
	if (newFD < 0)
		return false;

	if ( insertFD_(newFD) ){
		// socket_makeNonBlocking(newFD);

		log_("Connect", newFD);
	}else{
		socket_close(newFD);

		log_("Drop connect", newFD);
	}

	return true;
}

template<class SELECTOR, class CONNECTION>
void AsyncLoop<SELECTOR, CONNECTION>::handleDisconnect_(int const fd, const DisconnecStatus &error){
	removeFD_(fd);

	socket_close(fd);

	switch(error){
	case DisconnecStatus::NORMAL	: return log_("Normal  Disconnect",  fd);
	case DisconnecStatus::ERROR	: return log_("Error   Disconnect",  fd);
	case DisconnecStatus::PROBLEM	: return log_("Problem Disconnect",  fd);
	case DisconnecStatus::TIMEOUT	: return log_("Timeout Disconnect",  fd);
	};
}

// ===========================

template<class SELECTOR, class CONNECTION>
bool AsyncLoop<SELECTOR, CONNECTION>::insertFD_(int const fd){
	// one for server fd
	if (_connectedClients + 1 >= _selector.maxFD() )
		return false;

	bool const result = _selector.insertFD(fd);

	if (result == false)
		return false;

	_connections.emplace(fd, fd);

	++_connectedClients;

	return true;
}

template<class SELECTOR, class CONNECTION>
void AsyncLoop<SELECTOR, CONNECTION>::removeFD_(int const fd){
	_selector.removeFD(fd);

	_connections.erase(fd);

	--_connectedClients;
}

template<class SELECTOR, class CONNECTION>
void AsyncLoop<SELECTOR, CONNECTION>::_expireFD(){
	for(const auto &p : _connections){
		auto &c = p.second;

		if (c.expired(CONN_TIMEOUT)){
			handleDisconnect_(c.fd, DisconnecStatus::TIMEOUT);
			// iterator is invalid now...
			return;
		}
	}
}

} // namespace

