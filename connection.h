#ifndef _NET_CONNECTION_H
#define _NET_CONNECTION_H

#include "time.h"

#include <cstdlib>

namespace net{

	template<size_t BUFFER_SIZE = 1024>
	class Connection{
	public:
		Connection(int const fd) : fd(fd){}

		constexpr static size_t buffer_max = BUFFER_SIZE;

	public:
		int		fd;
		uint32_t	time			= now();
		uint16_t	buffer_size		= 0;
		char		buffer[buffer_max];

	public:
		bool expired(uint32_t timeout) const{
			return time + timeout < now();
		}

		void refresh() {
			time = now();
		}
	};

} // namespace

#endif

