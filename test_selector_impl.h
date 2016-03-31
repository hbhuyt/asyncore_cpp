
// selector definiton already available here.

#include "sockets.h"
#include "asyncloop.h"
#include "redisprotocol.h"

int main(){
	using MySelector = MySelector;
	using MyProtocol = protocol::RedisProtocol;



	int const fd = net::socket_create("localhost.not.used.yet", 2000);

	net::AsyncLoop<MySelector, MyProtocol> loop( MySelector{ 4 }, MyProtocol{}, fd );

	while(loop.process());
}

