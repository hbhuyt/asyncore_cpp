#ifndef _REDIS_PROTOCOL_H
#define _REDIS_PROTOCOL_H

#include "stringref.h"
#include "checkedvalue.h"

#include "protocoldefs.h"

#include <vector>

namespace net{
namespace protocol{

class RedisProtocol{
public:
	using Status = ProtocolStatus;

public:
	RedisProtocol(){
		params_.reserve(MAX_PARAMS);
	}

public:
	Status operator()(const StringRef &src);

	const std::vector<StringRef> &getParams() const{
		return params_;
	}

	void print() const;

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

public:
	static const StringRef	RESPONSE_OK;
	static const StringRef	RESPONSE_ERROR;

	static const StringRef	CMD_SHUTDOWN;

private:
	constexpr static char	STAR		= '*';
	constexpr static char	DOLLAR		= '$';

	constexpr static size_t	INT_BUFFER_SIZE	= 8;	// to be able to store MAX_PARAM_SIZE as string.
	constexpr static size_t	MAX_PARAMS	= 4;	// setex name 100 hello
	constexpr static size_t	MAX_PARAM_SIZE	= 16 * 1024;

private:
	std::vector<StringRef>	params_;
};


} // namespace protocol
} // namespace

#endif

