#ifndef _NET_CONNECTION_H
#define _NET_CONNECTION_H

#include "time.h"

#include "stringref.h"

#include <cstring>


namespace net{

	template<size_t BUFFER_SIZE>
	class Connection{
	public:
		Connection(int const fd) : fd(fd){}

		constexpr static size_t buffer_max = BUFFER_SIZE;

		using size_type = uint16_t;

	public:
		int		fd;
		uint32_t	time			= now();
		size_type	buffer_start		= 0;
		size_type	buffer_size		= 0;
		char		buffer[buffer_max];

	public:
		bool expired(uint32_t timeout) const{
			return time + timeout < now();
		}

		void refresh() {
			time = now();
		}

		void clear(){
			buffer_start = 0;
			buffer_size  = 0;
		}

		bool setData(const void *data, size_t const size){
			if (size > buffer_max){
				clear();
				return false;
			}

			buffer_size = (size_type) size;
			memcpy(buffer, data, size);

			return true;
		}

		const StringRef getStringRef() const{
			return { buffer, buffer_size };
		};
	};

} // namespace

#endif

