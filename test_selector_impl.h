
// selector definiton already available here.

#include "sockets.h"
#include "asyncloop.h"

int main(){
	using MySelector = MySelector;
	using MyWorker   = MyWorker;

	int const fd = net::socket_create("localhost.not.used.yet", 2000);

	net::AsyncLoop<MySelector, MyWorker> loop( MySelector{ 4 }, MyWorker{}, fd );

	while(loop.process());
}

