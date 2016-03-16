#include "sockets.h"
#include "asyncloop.h"
#include "epollselector.h"

int main(){
	using net::AsyncLoop;
	using MySelector = net::EPollSelector;

	int const fd = net::socket_create("localhost.not.used.yet", 2000);

	AsyncLoop<MySelector> loop( MySelector{ 4 }, fd );

	while(loop.process());
}

