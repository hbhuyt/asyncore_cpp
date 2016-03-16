MYCC	= clang
MYCC	= g++

CXX_OPT	= -g
CXX_OPT	= -O9
CXX_OPT	=

CXX	= $(MYCC) -std=c++11 -Wall  -Wconversion 	\
		-MMD -MP		\
		$(CXX_OPT)		\
		-c

#		-D_FILE_OFFSET_BITS=64	\

LINK	= $(MYCC) -o
LIBS	= -lstdc++

#-ljemalloc
#-fpack-struct

SRC	= $(wildcard *.cc)


TARGETS	=	test_poll test_epoll


all: $(TARGETS)


clean:
	rm -f *.o *.d *.gch		\
			$(TARGETS)


test_poll: test_poll.o time.o sockets.o pollselector.o
	$(LINK) $@ $^			$(LIBS)


test_epoll: test_epoll.o time.o sockets.o epollselector.o
	$(LINK) $@ $^			$(LIBS)


%.o: %.cc
	$(CXX) $<

-include $(SRC:%.cc=%.d)

