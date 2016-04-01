#ifndef _ECHO_WORKER_H
#define _ECHO_WORKER_H

#include "workerdefs.h"

#include <cstdlib>

namespace net{

class EchoWorker{
public:
	template<class CONNECTION>
	WorkerStatus operator()(CONNECTION &buffer);

private:
	constexpr static const char *cmd_helo = "hello\r\n";
	constexpr static const char *cmd_exit = "exit\r\n";
};

} // namespace

// ==================================

#include "echoworker_impl.h"

#endif
