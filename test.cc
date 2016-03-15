#include "mysocket.h"
#include "asyncloop.h"
#include "asyncpollselector.h"

int main(){
	int fd = MySocket::create("localhost.not.used.yet", 2000);

	Async::Loop<Async::PollSelector> loop( Async::PollSelector{ 4 }, fd );

	while(loop.process());
}

