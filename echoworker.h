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
	constexpr static char cmd_hello[]	= "hello\r\n";
	constexpr static char cmd_help[]	= "help\r\n";
	constexpr static char cmd_exit[]	= "exit\r\n";
	constexpr static char cmd_shutdown[]	= "shutdown\r\n";

	constexpr static char msg_help[]	=
				"Usage:\r\n"
				"   hello    - greeting\r\n"
				"   help     - this message\r\n"
				"   exit     - disconnect\r\n"
				"   shutdown - shutdown the server\r\n"
				"\r\n";

private:
	template<size_t N, class CONNECTION>
	constexpr static bool cmp_(const char (&cmd)[N], const CONNECTION &b){
		return N - 1 == b.size() && strncmp(cmd, b.data(), N - 1) == 0;
	}

};

} // namespace

// ==================================

#include "echoworker_impl.h"

#endif
