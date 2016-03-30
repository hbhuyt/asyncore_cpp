#include "redisprotocol.h"

#include <iostream>

const char *test_status(const Protocol::Status &status){
	using Status = Protocol::Status;

	switch(status){
	case Status::OK			: return "OK"		;
	case Status::BUFFER_NOT_READ	: return "MORE"		;
	default:
	case Status::ERROR		: return "ERROR"	;
	}
}

template <class PROTOCOL>
void test(PROTOCOL &&p, const char *data){
	using Status = Protocol::Status;

	const Status status = p(data);

	std::cout	<< "Result status: " << test_status(status) << std::endl
			<< std::endl
	;

	if (status == Status::OK)
		p.dump();

	std::cout	<< "---(eof)---" << std::endl
			<< std::endl
	;
}


int main(){
	Protocol::RedisProtocol p;

	test(p, "*3\r\n$3\r\nSET\r\n$4\r\ncity\r\n$5\r\nSofia\r\n");

	test(p, "*2\r\n$3\r\nGET\r\n$4\r\ncity\r\n");

	test(p, "*92\r\n$3\r\nGET\r\n$4\r\ncity\r\n");
	test(p, "$2\r\n$3\r\nGET\r\n$4\r\ncity\r\n");
	test(p, "*2\r\n*3\r\nGET\r\n$4\r\ncity\r\n");

#if 0
	using Status = Protocol::Status;

	const char *data = "*3\r\n$3\r\nSET\r\n$4\r\ncity\r\n$5\r\nSofia\r\n";

	for(int i = 0; i < 2500000; ++i){
		const auto &status = p(data);

		if (status == Status::BUFFER_NOT_READ)
			std::cout << "problem" << std::endl;
	}
#endif
}
