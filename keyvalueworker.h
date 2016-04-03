#ifndef _KEY_VALUE_WORKER_H
#define _KEY_VALUE_WORKER_H

#include "workerdefs.h"


namespace net{
namespace worker{


template<class PROTOCOL>
class KeyValueWorker{
public:
	template<class CONNECTION>
	WorkerStatus operator()(CONNECTION &buffer);

private:
	template<class CONNECTION>
	WorkerStatus process_request_(CONNECTION &buffer);

private:
	PROTOCOL protocol_;
};


} // namespace worker
} // namespace


// ==================================

#include "keyvalueworker_impl.h"

#endif
