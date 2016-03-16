#include "sockets.h"
#include "asyncloop.h"

#ifndef NET_ALTERNATIVE_SELECTOR

#include "pollselector.h"

using MySelector = net::PollSelector;

#endif


int main(){
	int const fd = net::socket_create("localhost.not.used.yet", 2000);

	net::AsyncLoop<MySelector> loop( MySelector{ 4 }, fd );

	while(loop.process());
}

