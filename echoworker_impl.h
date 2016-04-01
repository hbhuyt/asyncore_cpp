
#include <cstring>

namespace net{

template<class CONNECTION>
WorkerStatus EchoWorker::operator()(CONNECTION &buffer){
	static size_t const cmd_exit_len = strlen(cmd_exit);
	static size_t const cmd_helo_len = strlen(cmd_helo);

	if (buffer.size() == 0)
		return WorkerStatus::BUFFER_NOT_READ;

	if (buffer.size() == cmd_exit_len && strncmp(buffer.data(), cmd_exit, cmd_exit_len) == 0){
		return WorkerStatus::DISCONNECT;
	}

	if (buffer.size() == cmd_helo_len && strncmp(buffer.data(), cmd_helo, cmd_helo_len) == 0){
		buffer.clear();
		buffer.push("Hello, how are you?\r\n");
		return WorkerStatus::WRITE;
	}

	return WorkerStatus::WRITE;
}

} // namespace

