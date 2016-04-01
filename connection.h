#ifndef _NET_CONNECTION_H
#define _NET_CONNECTION_H

#include "connectionbuffer.h"

#include "time.h"

#include <cstdlib>

namespace net{
	template<size_t BUFFER_SIZE>
	class Connection{
	public:
		Connection(int const fd) : fd_(fd){}

	public:
		using Buffer = ConnectionBuffer<BUFFER_SIZE>;

	private:
		int		fd_;
		uint32_t	time_		= now();
	public:
		Buffer		buffer;

	public:
		int fd() const{
			return fd_;
		}

		bool expired(uint32_t timeout) const{
			return time_ + timeout < now();
		}

		void restartTimer() {
			time_ = now();
		}
	};

} // namespace

#endif

