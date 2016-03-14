#include "mysocket.h"
#include "asyncloop.h"
#include "pollasyncselector.h"

int main(){
	int fd = MySocket::create("localhost.not.used.yet", 2000);

	AsyncLoop<PollAsyncSelector> loop( PollAsyncSelector{ 10000 }, fd );

	while(loop.process());
}

