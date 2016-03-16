
int main(){
	int const fd = net::socket_create("localhost.not.used.yet", 2000);

	net::AsyncLoop<MySelector> loop( MySelector{ 4 }, fd );

	while(loop.process());
}

