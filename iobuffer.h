#ifndef _IO_BUFFER_H
#define _IO_BUFFER_H

#include <cstdint>
#include <cstring>
#include <cstdio>

#include <unistd.h>	// read

namespace net{

template<size_t CAPACITY>
class IOBuffer{
public:
	using size_type = uint16_t;

private:
	size_type	head_	= 0;
	size_type	tail_	= 0;
	char		buffer_[CAPACITY];

public:
	void clear(){
		head_ = 0;
		tail_ = 0;
	}

	constexpr
	static size_t max_size(){
		return CAPACITY;
	}

	// ==================================

	const char *data() const{
		return & buffer_[head_];
	}

	char *dataTail(){
		return & buffer_[tail_];
	}

	size_t size() const{
		return tail_ - head_;
	}

	size_t capacity() const{
		return CAPACITY - tail_;
	}

	// ==================================

	bool push(const char *p){
		return p ? push(strlen(p), p) : false;
	}

	bool push(size_t const len, const void *ptr = nullptr){
		if (len == 0)
			return false;

		if (capacity() < len)
			return false;

		if (ptr)
			memmove(&buffer_[tail_], ptr, len);

		tail_ = (size_type) (tail_ + len);

		return true;
	}

	bool pop(size_t const len){
		if (len == 0)
			return false;

		auto const available = size();

		if (available < len)
			return false;

		if (available == len){
			clear();
			return true;
		}

		head_ = (size_type) (head_ + len);

		return true;
	}

	// ==================================

	void print() const{
		printf("h: %3u | t: %3u | ad: %3zu | ac: %3zu | %.*s\n",
				head_, tail_,
				size(), capacity(),
				(int) size(), buffer_ );
	}
};

} // namespace

#endif

