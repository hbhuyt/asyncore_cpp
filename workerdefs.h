#ifndef _WORKER_DEFS_H
#define _WORKER_DEFS_H

namespace net{

	enum class WorkerStatus{
		BUFFER_NOT_READ,
		READ,
		WRITE,
		DISCONNECT,
		DISCONNECT_ERROR,
	};

} // namespace

#endif
