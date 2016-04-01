#ifndef _NET_BUFFER_H
#define _NET_BUFFER_H

#include <cstdint>
#include <cstring>

#include <unistd.h>	// read

namespace net{

	template<size_t BUFFER_SIZE>
	struct ConnectionBuffer{
		using size_type = uint16_t;

		constexpr static size_type max_size = BUFFER_SIZE;

		// ==================================

		char		data[max_size];
		size_type	start			= 0;
		size_type	size			= 0;

		// ==================================

		void clear(){
			start = 0;
			size  = 0;
		}

		bool set(const void *p, size_t const s){
			if (p == nullptr || s > max_size){
				clear();
				return false;
			}

			size = (size_type) s;
			memcpy(data, p, s);

			return true;
		}

		// ==================================

		ssize_t availableReadSize() const{
			return max_size - size;
		}

		ssize_t read(int const fd){
			ssize_t const available = availableReadSize();

			if (available < 0)
				return -1;

			if (available == 0)
				return 0;

			ssize_t const count = ::read(fd, & data[size], available);

			if (count > 0)
				size = (size_type) (size + count);

			return count;
		}

		// ==================================

		ssize_t availableWriteSize() const{
			return size - start;
		}

		ssize_t write(int const fd){
			ssize_t const available = availableWriteSize();

			if (available < 0)
				return -1;

			if (available == 0)
				return 0;

			ssize_t const count = ::write(fd, & data[start], available);

			if (count == available)
				clear();
			else if (count > 0)
				start = (size_type) (start + count);

			return count;
		}
	};

} // namespace

#endif

