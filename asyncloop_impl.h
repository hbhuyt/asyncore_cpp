#include "sockets.h"

#include "stringref.h"

#include <unistd.h>	// read

namespace net{

template<class SELECTOR, class PROTOCOL, class CONNECTION>
AsyncLoop<SELECTOR, PROTOCOL, CONNECTION>::AsyncLoop(SELECTOR &&selector, PROTOCOL &&protocol, int const serverFD) :
					selector_(std::move(selector)),
					protocol_(std::move(protocol)),
					serverFD_(serverFD){
	socket_makeNonBlocking(serverFD_);
	selector_.insertFD(serverFD_);
}

template<class SELECTOR, class PROTOCOL, class CONNECTION>
AsyncLoop<SELECTOR, PROTOCOL, CONNECTION>::~AsyncLoop(){
	// serverFD_ will be closed if we close epoll
	selector_.removeFD(serverFD_);
}

// ===========================

template<class SELECTOR, class PROTOCOL, class CONNECTION>
bool AsyncLoop<SELECTOR, PROTOCOL, CONNECTION>::process(){
	using WaitStatus = selector::WaitStatus;
	using FDStatus   = selector::FDStatus;

	log_("poll()-ing...");
	const WaitStatus status = selector_.wait(WAIT_TIMEOUT_MS);

	if (status == WaitStatus::ERROR){
		log_("poll() error");

		return false;
	}

	if (status == WaitStatus::NONE){
		// idle loop, check for expired conn
		expireFD_();
		return true;
	}

	for(uint32_t i = 0; i < selector_.maxFD(); ++i){
		const auto &t = selector_.getFDStatus(i);

		switch(t.status){
		case FDStatus::READ:
			handleRead_( t.fd );
			break;

		case FDStatus::WRITE:
			handleWrite_( t.fd );
			break;

		case FDStatus::ERROR:
			handleDisconnect_( t.fd, DisconnectStatus::ERROR );
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

template<class SELECTOR, class PROTOCOL, class CONNECTION>
void AsyncLoop<SELECTOR, PROTOCOL, CONNECTION>::handleRead_(int const fd){
	if (fd == serverFD_){
		while (handleConnect_(fd));
		return;
	}

	// -------------------------------------

	auto it = connections_.find(fd);

	if (it == connections_.end())
		return handleDisconnect_(fd, DisconnectStatus::PROBLEM_MAP_NOT_FOUND);

	auto &connection = it->second;
	auto &buffer = connection.buffer;

	// -------------------------------------

	if (buffer.availableReadSize() <= 0){
		// buffer will overfow, disconnect.
		return handleDisconnect_(fd, DisconnectStatus::PROBLEM_BUFFER_READ);
	}

	ssize_t const size = buffer.read(fd);

	if (size <= 0)
		return handleSocketOps_(fd, size);

	connection.restartTimer();

	handleProtocol_(fd, connection);
}

template<class SELECTOR, class PROTOCOL, class CONNECTION>
void AsyncLoop<SELECTOR, PROTOCOL, CONNECTION>::handleWrite_(int const fd){
	if (fd == serverFD_){
		// WTF?!?
		return;
	}

	// -------------------------------------

	auto it = connections_.find(fd);

	if (it == connections_.end())
		return handleDisconnect_(fd, DisconnectStatus::PROBLEM_MAP_NOT_FOUND);

	auto &connection = it->second;
	auto &buffer = connection.buffer;

	// -------------------------------------

	if (buffer.availableWriteSize() <= 0){
		// this could never happen, disconnect.
		return handleDisconnect_(fd, DisconnectStatus::PROBLEM_BUFFER_WRITE);
	}

	ssize_t const size = buffer.write(fd);

	// -------------------------------------

	if (size <= 0)
		return handleSocketOps_(fd, size);

	connection.restartTimer();

	if (buffer.availableWriteSize() == 0){
		// process with read
		selector_.updateFD(fd, selector::FDEvent::READ);

		return;
	}
}

template<class SELECTOR, class PROTOCOL, class CONNECTION>
bool AsyncLoop<SELECTOR, PROTOCOL, CONNECTION>::handleProtocol_(int const fd, CONNECTION &connection){
	static const char *redis_ok  = "$2\r\nOK\r\n";
	static const char *redis_err = "-ERR Error\r\n";

	using Status = ProtocolStatus;

	const StringRef sr{ connection.buffer.data, connection.buffer.size };

	const Status status = protocol_( sr );

	switch( status ){
	case Status::BUFFER_NOT_READ:
		// need more data

		return false;

	case Status::OK:
		// OK responce

		printf("Dump data from protocol...\n");
		protocol_.print();

		connection.buffer.set(redis_ok, strlen(redis_ok));

		selector_.updateFD(fd, selector::FDEvent::WRITE);

		return true;

	default:
	case Status::ERROR:
		// ERR responce

		printf("ERROR IN PROTOCOL...\n");

		connection.buffer.set(redis_err, strlen(redis_err));

		selector_.updateFD(fd, selector::FDEvent::WRITE);

		return true;
	}
}

template<class SELECTOR, class PROTOCOL, class CONNECTION>
bool AsyncLoop<SELECTOR, PROTOCOL, CONNECTION>::handleConnect_(int const fd){
	// fd is same as serverFD_
	int const newFD = socket_accept(fd);

	// serverFD_ is non blocking, so we do not need to check EAGAIN
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

template<class SELECTOR, class PROTOCOL, class CONNECTION>
void AsyncLoop<SELECTOR, PROTOCOL, CONNECTION>::handleDisconnect_(int const fd, const DisconnectStatus error){
	removeFD_(fd);

	socket_close(fd);

	switch(error){
	case DisconnectStatus::NORMAL			: return log_("Normal  Disconnect",				fd);
	case DisconnectStatus::ERROR			: return log_("Error   Disconnect",				fd);
	case DisconnectStatus::TIMEOUT			: return log_("Timeout Disconnect",				fd);

	case DisconnectStatus::PROBLEM_MAP_NOT_FOUND	: return log_("Problem Disconnect - FD not found",		fd);
	case DisconnectStatus::PROBLEM_BUFFER_READ	: return log_("Problem Disconnect - Read buffer full",		fd);
	case DisconnectStatus::PROBLEM_BUFFER_WRITE	: return log_("Problem Disconnect - Write buffer problem",	fd);
	};
}

// ===========================

template<class SELECTOR, class PROTOCOL, class CONNECTION>
void AsyncLoop<SELECTOR, PROTOCOL, CONNECTION>::handleSocketOps_(int const fd, ssize_t const size){
	if (size < 0){
		if ( socket_check_eagain() ){
			// try again
			return;
		}else{
			// error, disconnect.
			return handleDisconnect_(fd, DisconnectStatus::ERROR);
		}
	}

	if (size == 0){
		// normal, disconnect.
		return handleDisconnect_(fd, DisconnectStatus::NORMAL);
	}
}

// ===========================

template<class SELECTOR, class PROTOCOL, class CONNECTION>
bool AsyncLoop<SELECTOR, PROTOCOL, CONNECTION>::insertFD_(int const fd){
	// one for server fd
	if (connectedClients_ + 1 >= selector_.maxFD() )
		return false;

	bool const result = selector_.insertFD(fd);

	if (result == false)
		return false;

	connections_.emplace(fd, fd);

	++connectedClients_;

	return true;
}

template<class SELECTOR, class PROTOCOL, class CONNECTION>
void AsyncLoop<SELECTOR, PROTOCOL, CONNECTION>::removeFD_(int const fd){
	selector_.removeFD(fd);

	connections_.erase(fd);

	--connectedClients_;
}

template<class SELECTOR, class PROTOCOL, class CONNECTION>
void AsyncLoop<SELECTOR, PROTOCOL, CONNECTION>::expireFD_(){
	for(const auto &p : connections_){
		auto &c = p.second;

		if (c.expired(CONN_TIMEOUT)){
			handleDisconnect_(c.fd(), DisconnectStatus::TIMEOUT);
			// iterator is invalid now...
			return;
		}
	}
}

} // namespace

