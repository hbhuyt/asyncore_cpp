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


TARGETS	=	test_poll		\
		test_iobuffer	\
		test_redisprotocol


ifeq ($(UNAME), Linux)
	# add epoll support...
	TARGETS	+= test_epoll
endif


all: $(TARGETS)


clean:
	rm -f *.o *.d *.gch		\
			$(TARGETS)


test_poll: test_poll.o pollselector.o		time.o sockets.o echoworker.o
	$(LINK) $@ $^			$(LIBS)

test_epoll: test_epoll.o epollselector.o	time.o sockets.o echoworker.o
	$(LINK) $@ $^			$(LIBS)

test_iobuffer: test_iobuffer.o
	$(LINK) $@ $^			$(LIBS)

test_redisprotocol: test_redisprotocol.o redisprotocol.o
	$(LINK) $@ $^			$(LIBS)

%.o: %.cc
	$(CXX) $<

-include $(SRC:%.cc=%.d)

