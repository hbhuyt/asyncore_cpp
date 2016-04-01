#ifndef _NET_CONNECTION_H
#define _NET_CONNECTION_H

#include "iobuffer.h"

#include "time.h"

#include <cstdlib>

namespace net{

template<size_t CAPACITY>
class ClientBuffer : public IOBuffer<CAPACITY>{
private:
	uint32_t	time_		= now();

public:
	bool expired(uint32_t timeout) const{
		return time_ + timeout < now();
	}

	void restartTimer() {
		time_ = now();
	}
};

} // namespace

#endif

