#ifndef _ASYNC_DEFS_H
#define _ASYNC_DEFS_H

#include <chrono>

namespace Async{

enum class WaitStatus{ OK, NONE, ERROR };
enum class FDStatus{ NONE, READ, WRITE, ERROR };

// ===========================

static uint32_t _now();

// ===========================

template<size_t BUFFER_SIZE = 1024>
class Connection{
public:
	Connection(int const fd) : fd(fd), time(_now()){}

	constexpr static size_t buffer_max = BUFFER_SIZE;

	int		fd;
	uint32_t	time;
	uint16_t	buffer_size = 0;
	char		buffer[buffer_max];
};

// ===========================

template<class CONNECTION>
inline bool expired(const CONNECTION &conn, uint32_t timeout){
	return conn.time + timeout < _now();
}

template<class CONNECTION>
inline void refresh(CONNECTION &conn) {
	conn.time = _now();
}

// ===========================

static uint32_t _now(){
	const auto now = std::chrono::system_clock::now().time_since_epoch();

	const auto sec = std::chrono::duration_cast<std::chrono::seconds>(now);

	return (uint32_t) sec.count();
}

}; // namespace

#endif

