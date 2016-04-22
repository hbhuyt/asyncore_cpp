#include "pollselector.h"
#include "keyvalueworker.h"
#include "redisprotocol.h"

using MySelector	= net::selector::PollSelector;
using MyProtocol	= net::protocol::RedisProtocol;
using MyWorker		= net::worker::KeyValueWorker<MyProtocol>;

#include "test_selector_impl.h"

