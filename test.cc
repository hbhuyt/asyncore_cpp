#include "mysocket.h"
#include "pollasyncloop.h"

int main(){
	int fd = MySocket::create(nullptr, 2000);

	PollAsyncLoop loop(10000, fd);

	while(loop.wait());
}

