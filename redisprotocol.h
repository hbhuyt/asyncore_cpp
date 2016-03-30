#ifndef _REDIS_PROTOCOL_H
#define _REDIS_PROTOCOL_H

#include "stringref.h"
#include "checkedvalue.h"

#include "protocoldefs.h"

#include <vector>
#include <iostream>

namespace Protocol{


class RedisProtocol{
public:
	RedisProtocol(){
		params_.reserve(MAX_PARAMS);
	}

public:
	Status operator()(const StringRef &src);

	void dump(std::ostream &os = std::cout) const{
		for(const auto &item : params_ )
			os << item << std::endl;
	}

private:
	static int readInt_(const StringRef &src, size_t &pos);
	static Status readLn_(const StringRef &src, size_t &pos);

	using CheckedStringref = CheckedValue<StringRef, Status, Status::OK>;

	static CheckedStringref readParam_(const StringRef &src, size_t &pos);

private:
	enum class Error;

	constexpr static Status err_(const Error &){
		return Status::ERROR;
	}

private:
	constexpr static char	STAR		= '*';
	constexpr static char	DOLLAR		= '$';

	constexpr static size_t	INT_BUFFER_SIZE	= 8;	// to be able to store MAX_PARAM_SIZE as string.
	constexpr static size_t	MAX_PARAMS	= 4;	// setex name 100 hello
	constexpr static size_t	MAX_PARAM_SIZE	= 16 * 1024;

private:
	std::vector<StringRef>	params_;
};


} // namespace

#endif

