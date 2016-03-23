#include "rediscpp.h"


enum class RedisProtocol::RedisProtocolError{
	NO_STAR,
	NO_DOLLAR,
	PARAM_COUNT,
	PARAM_SIZE,
	NO_CRLF
};


ProtocolStatus RedisProtocol::operator()(const StringRef &src){
	if (src.size() < 8)	// 4 bytes - "*1\r\n$1\r\n"
		return ProtocolStatus::BUFFER_NOT_READ;

	size_t pos = 0;

	if (src[pos] != STAR)
		return __err(RedisProtocolError::NO_STAR);

	++pos;

	if (pos + 1 > src.size())
		// at least one character for parameter count
		return ProtocolStatus::BUFFER_NOT_READ;

	paramsCount = _readInt(src, pos);

	if (paramsCount == 0 || paramsCount > MAX_PARAMS)
		return __err(RedisProtocolError::PARAM_COUNT);

	{
		ProtocolStatus stat = _readLn(src, pos);
		if ( stat != ProtocolStatus::OK )
			return stat;
	}

	for(size_t i = 0; i < paramsCount; ++i){
		ProtocolStatus stat = _readParam(src, pos, params[i]);
		if (stat != ProtocolStatus::OK )
			return stat;
	}

	return ProtocolStatus::OK;
}


int RedisProtocol::_readInt(const StringRef &src, size_t &pos){
	char buff[INT_BUFFER_SIZE + 1];

	size_t bpos = 0;

	while(pos < src.size() && bpos < INT_BUFFER_SIZE){
		char const c = src[pos];
		if (c != '-' && c != '+' && (c < '0' || c > '9') )
			break;

		buff[bpos] = c;

		++pos;
		++bpos;
	}

	buff[bpos] = '\0';

	return atoi(buff);
}


ProtocolStatus RedisProtocol::_readLn(const StringRef &src, size_t &pos){
	if (pos + 1 > src.size())
		return ProtocolStatus::BUFFER_NOT_READ;

	if (src[pos] == '\r' && src[pos + 1] == '\n'){
		pos += 2;
		return ProtocolStatus::OK;
	}

	return __err(RedisProtocolError::NO_CRLF);
}


ProtocolStatus RedisProtocol::_readParam(const StringRef &src, size_t &pos, StringRef &store){
	if (pos + 1 > src.size())
		return ProtocolStatus::BUFFER_NOT_READ;

	if (src[pos] != DOLLAR)
		return __err(RedisProtocolError::NO_DOLLAR);

	++pos;

	if (pos + 1 > src.size())
		return ProtocolStatus::BUFFER_NOT_READ;

	const auto size = _readInt(src, pos);
	if (size <= 0 || size > MAX_PARAM_SIZE)
		return __err(RedisProtocolError::PARAM_SIZE);

	{
		ProtocolStatus stat = _readLn(src, pos);
		if ( stat != ProtocolStatus::OK )
			return stat;
	}

	const auto pos_save = pos;

	// store param must be here, but is optimized at the end.

	pos += size;

	// check if all data is there.
	if (pos > src.size())
		return ProtocolStatus::BUFFER_NOT_READ;

	{
		ProtocolStatus stat = _readLn(src, pos);
		if ( stat != ProtocolStatus::OK )
			return stat;
	}

	store = StringRef( & src[pos_save], size);

	return ProtocolStatus::OK;
}





const char *test_status(const ProtocolStatus &status){
	switch(status){
	case ProtocolStatus::OK			: return "OK"		;
	case ProtocolStatus::BUFFER_NOT_READ	: return "MORE"		;
	default:
	case ProtocolStatus::ERROR		: return "ERROR"	;
	}
}

template <class Protocol>
void test(Protocol &&p, const char *data){
	const ProtocolStatus status = p(data);

	std::cout	<< "Result status: " << test_status(status) << std::endl
			<< std::endl
	;

	if (status == ProtocolStatus::OK)
		p.dump();

	std::cout	<< "---(eof)---" << std::endl
			<< std::endl
	;
}

int main(){
	RedisProtocol p;

	test(p, "*3\r\n$3\r\nSET\r\n$4\r\ncity\r\n$5\r\nSofia\r\n");

	test(p, "*2\r\n$3\r\nGET\r\n$4\r\ncity\r\n");

	test(p, "*92\r\n$3\r\nGET\r\n$4\r\ncity\r\n");
	test(p, "$2\r\n$3\r\nGET\r\n$4\r\ncity\r\n");
	test(p, "*2\r\n*3\r\nGET\r\n$4\r\ncity\r\n");

#if 0
	const char *data = "*3\r\n$3\r\nSET\r\n$4\r\ncity\r\n$5\r\nSofia\r\n";

	for(int i = 0; i < 2500000; ++i){
		const ProtocolStatus status = p(data);

		if (status == ProtocolStatus::BUFFER_NOT_READ)
			std::cout << "problem" << std::endl;
	}
#endif
}

