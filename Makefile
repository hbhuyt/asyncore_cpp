MYCC	= clang
MYCC	= g++

CXX_OPT	= -g
CXX_OPT	= -O9
CXX_OPT	=

CXX	= $(MYCC) -std=c++11 -Wall -Wconversion -Wpedantic 	\
		-MMD -MP		\
		$(CXX_OPT)		\
		-c

#		-D_FILE_OFFSET_BITS=64	\

LINK	= $(MYCC) -o
LIBS	= -lstdc++

#-ljemalloc
#-fpack-struct

SRC	= $(wildcard *.cc)
UNAME	= $(shell uname -s)


TARGETS	=	test_echo_poll test_redis_poll		\
		test_iobuffer				\
		test_redisprotocol


ifeq ($(UNAME), Linux)
	# add epoll support...
	TARGETS	+= test_echo_epoll test_redis_epoll
endif


all: $(TARGETS)


clean:
	rm -f *.o *.d *.gch		\
			$(TARGETS)



test_echo_poll: test_echo_poll.o	pollselector.o	echoworker.o	timer.o sockets.o
	$(LINK) $@ $^			$(LIBS)

test_echo_epoll: test_echo_epoll.o	epollselector.o	echoworker.o	timer.o sockets.o
	$(LINK) $@ $^			$(LIBS)



test_redis_poll: test_redis_poll.o	pollselector.o	redisprotocol.o	timer.o sockets.o
	$(LINK) $@ $^			$(LIBS)

test_redis_epoll: test_redis_epoll.o	epollselector.o	redisprotocol.o	timer.o sockets.o
	$(LINK) $@ $^			$(LIBS)



test_iobuffer: test_iobuffer.o
	$(LINK) $@ $^			$(LIBS)

test_redisprotocol: test_redisprotocol.o redisprotocol.o
	$(LINK) $@ $^			$(LIBS)

%.o: %.cc
	$(CXX) $<

-include $(SRC:%.cc=%.d)

