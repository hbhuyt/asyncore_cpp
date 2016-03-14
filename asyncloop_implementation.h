#include <sys/socket.h>	// accept
#include <unistd.h>	// read, close
#include <fcntl.h>	// fcntl

#include <chrono>

template<class SELECTOR>
uint32_t AsyncLoop<SELECTOR>::Connection::now(){
	const auto now = std::chrono::system_clock::now().time_since_epoch();

	const auto sec = std::chrono::duration_cast<std::chrono::seconds>(now);

	return (uint32_t) sec.count();
}

// ===========================

template<class SELECTOR>
AsyncLoop<SELECTOR>::AsyncLoop(SELECTOR &&selector, int const serverFD) :
					_selector(std::move(selector)),
					_serverFD(serverFD){
	_selector.insertFD(_serverFD);
}

template<class SELECTOR>
AsyncLoop<SELECTOR>::~AsyncLoop() = default;

// ===========================

template<class SELECTOR>
bool AsyncLoop<SELECTOR>::process(){
	__log("poll()-ing...", 0, _connectedClients);
	WaitStatus const status = _selector.wait(TIMEOUT);

	if (status == WaitStatus::ERROR){
		__log("poll() error", 0, _connectedClients);

		return false;
	}

	if (status == WaitStatus::NONE)
		return true;

	for(uint32_t i = 0; i < _selector.maxFD(); ++i){
		const auto &t = _selector.getFDStatus(i);

		switch(std::get<1>(t)){
		case FDStatus::READ:
			_handleRead( std::get<0>(t) );
			break;

		case FDStatus::ERROR:
			_handleDisconnect( std::get<0>(t) );
			break;

		case FDStatus::NONE:
		default:
			break;

		}
	}

	return true;
}

// ===========================

template<class SELECTOR>
void AsyncLoop<SELECTOR>::_handleRead(int const fd){
	constexpr size_t buffer_size = 1024;
	static char buffer[buffer_size];

	// -------------------------------------

	if (fd == _serverFD){
		_handleConnect(fd);
		return;
	}

	// -------------------------------------

	ssize_t const size = read(fd, buffer, buffer_size);

	if (size < 0){
		if ( socket__check_eagain() ){
			// try again
			return;
		}else{
			// error, disconnect.
			return _handleDisconnect(fd, true);
		}
	}

	if (size == 0){
		// normal, disconnect.
		return _handleDisconnect(fd);
	}

	printf("%5d | %5zu | [begin]%.*s[end]\n", fd, size, (int) size, buffer);
}

template<class SELECTOR>
bool AsyncLoop<SELECTOR>::_handleConnect(int const fd){
	// fd is same as _serverFD
	int const newFD = accept(fd, nullptr, nullptr);

	// _serverFD is non blocking, so we do not need to check EAGAIN
	if (newFD < 0)
		return false;

	if ( _selector.insertFD(newFD) ){
		socket__makeNonBlocking(newFD);

		++_connectedClients;

		__log("Connect", newFD, _connectedClients);
	}else{
		::close(newFD);

		__log("Drop connect", newFD, _connectedClients);
	}

	return true;
}

template<class SELECTOR>
void AsyncLoop<SELECTOR>::_handleDisconnect(int const fd, bool const error){
	_selector.removeFD(fd);

	--_connectedClients;

	if (error)
		__log("Error Disconnect",  fd, _connectedClients);
	else
		__log("Normal Disconnect", fd, _connectedClients);
}





// ===========================

template<class SELECTOR>
inline bool AsyncLoop<SELECTOR>::socket__check_eagain(){
	return errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK;
}

template<class SELECTOR>
inline bool AsyncLoop<SELECTOR>::socket__makeNonBlocking(int const fd){
	return fcntl(fd, F_SETFL, O_NONBLOCK) >= 0 ;
}

