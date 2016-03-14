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

template<class SELECTOR>
inline bool AsyncLoop<SELECTOR>::Connection::expired(uint32_t const timeout) const{
	return time + timeout < now();
}
		
// ===========================

template<class SELECTOR>
AsyncLoop<SELECTOR>::AsyncLoop(SELECTOR &&selector, int const serverFD) :
					_selector(std::move(selector)),
					_serverFD(serverFD){
	_selector.insertFD(_serverFD);
}

//template<class SELECTOR>
//AsyncLoop<SELECTOR>::~AsyncLoop() = default;

// ===========================

template<class SELECTOR>
bool AsyncLoop<SELECTOR>::process(){
	__log("poll()-ing...", 0, _connectedClients);
	const WaitStatus &status = _selector.wait(WAIT_TIMEOUT_MS);

	if (status == WaitStatus::ERROR){
		__log("poll() error", 0, _connectedClients);

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
			_handleRead( std::get<0>(t) );
			break;

		case FDStatus::ERROR:
			_handleDisconnect( std::get<0>(t), DisconnecStatus::ERROR );
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
			return _handleDisconnect(fd, DisconnecStatus::ERROR);
		}
	}

	if (size == 0){
		// normal, disconnect.
		return _handleDisconnect(fd, DisconnecStatus::NORMAL);
	}
		
	try{
		Connection &conn = _connections.at(fd);
		conn.refresh();

		printf("%5d | %5zu | [begin]%.*s[end]\n", fd, size, (int) size, buffer);
	}catch(const std::out_of_range& oor){
		// inconsistency, disconnect.
		return _handleDisconnect(fd, DisconnecStatus::PROBLEM);
	}
}

template<class SELECTOR>
bool AsyncLoop<SELECTOR>::_handleConnect(int const fd){
	// fd is same as _serverFD
	int const newFD = accept(fd, nullptr, nullptr);

	// _serverFD is non blocking, so we do not need to check EAGAIN
	if (newFD < 0)
		return false;

	if ( _insertFD(newFD) ){
		socket__makeNonBlocking(newFD);

		__log("Connect", newFD, _connectedClients);
	}else{
		::close(newFD);

		__log("Drop connect", newFD, _connectedClients);
	}

	return true;
}

template<class SELECTOR>
void AsyncLoop<SELECTOR>::_handleDisconnect(int const fd, const DisconnecStatus &error){
	_removeFD(fd);
	
	::close(fd);

	switch(error){
	case DisconnecStatus::NORMAL:	return __log("Normal  Disconnect",  fd, _connectedClients);
	case DisconnecStatus::ERROR:
	case DisconnecStatus::PROBLEM:	return __log("Error   Disconnect",  fd, _connectedClients);
	case DisconnecStatus::TIMEOUT:	return __log("Timeout Disconnect",  fd, _connectedClients);
	};
}

// ===========================

template<class SELECTOR>
bool AsyncLoop<SELECTOR>::_insertFD(int const fd){
	bool const result = _selector.insertFD(fd);

	if (result == false)
		return false;
	
	_connections.emplace(fd, fd);
	
	++_connectedClients;
	
	return true;
}

template<class SELECTOR>
void AsyncLoop<SELECTOR>::_removeFD(int const fd){
	_selector.removeFD(fd);
	
	_connections.erase(fd);

	--_connectedClients;
}

template<class SELECTOR>
void AsyncLoop<SELECTOR>::_expireFD(){
	for(auto &p : _connections){
		auto &conn = p.second;
		
		if (conn.expired(CONN_TIMEOUT)){
			_handleDisconnect(conn.fd, DisconnecStatus::TIMEOUT);
		//	break;
		}
	}
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

