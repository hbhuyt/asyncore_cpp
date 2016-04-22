
#include <cstring>

#include "protocoldefs.h"

#include "stringref.h"


namespace net{
namespace worker{


template<class PROTOCOL>
template<class CONNECTION>
WorkerStatus KeyValueWorker<PROTOCOL>::operator()(CONNECTION &buffer){
	if (buffer.size() == 0)
		return WorkerStatus::PASS;

	using ProtocolStatus = net::protocol::ProtocolStatus;

	auto status = protocol_( StringRef{ buffer.data(), buffer.size() } );

	switch(status){
		case ProtocolStatus::BUFFER_NOT_READ :
			return WorkerStatus::PASS;

		case ProtocolStatus::OK : {
			protocol_.print();

			return process_request_(buffer);
		}

		case ProtocolStatus::ERROR : {
			buffer.clear();
			buffer.push( PROTOCOL::RESPONSE_ERROR );
			return WorkerStatus::WRITE;
		}
	}

	return WorkerStatus::WRITE;
}

template<class PROTOCOL>
template<class CONNECTION>
WorkerStatus KeyValueWorker<PROTOCOL>::process_request_(CONNECTION &buffer){
	const auto &p = protocol_.getParams();

	if (p[0] == PROTOCOL::CMD_SHUTDOWN){
		return WorkerStatus::SHUTDOWN;
	}

	buffer.clear();
	buffer.push( PROTOCOL::RESPONSE_OK );
	return WorkerStatus::WRITE;
}


} // namespace worker
} // namespace

