#ifndef _REDIS_CPP_H
#define _REDIS_CPP_H

#include "stringref.h"

#include <iostream>


enum class ProtocolStatus{
	OK,
	BUFFER_NOT_READ,
	ERROR
};


class RedisProtocol{
public:
	ProtocolStatus operator()(const StringRef &src);

	void dump(std::ostream &os = std::cout) const{
		for(size_t i = 0; i < paramsCount; ++i)
			os << params[i] << std::endl;
	}

private:
	static int _readInt(const StringRef &src, size_t &pos);
	static ProtocolStatus _readLn(const StringRef &src, size_t &pos);
	static ProtocolStatus _readParam(const StringRef &src, size_t &pos, StringRef &store);

private:
	enum class RedisProtocolError;

	constexpr static ProtocolStatus __err(const RedisProtocolError &){
		return ProtocolStatus::ERROR;
	}

private:
	constexpr static char	STAR		= '*';
	constexpr static char	DOLLAR		= '$';

	constexpr static size_t	INT_BUFFER_SIZE	= 5;	// to be able to store MAX_PARAM_SIZE as string.
	constexpr static size_t MAX_PARAMS	= 4;	// setex name 100 hello
	constexpr static size_t MAX_PARAM_SIZE	= 16 * 1024;

private:
	size_t		paramsCount;
	StringRef	params[MAX_PARAMS];
};


#endif

