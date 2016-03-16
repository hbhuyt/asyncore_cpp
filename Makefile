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
UNAME	= $(shell uname -s)


TARGETS	=	test


ifeq ($(UNAME), Linux)
	# add epoll support...
	TARGETS	+= test_epoll
endif


all: $(TARGETS)


clean:
	rm -f *.o *.d *.gch		\
			$(TARGETS)


test: test.o time.o sockets.o pollselector.o
	$(LINK) $@ $^			$(LIBS)


test_epoll: test_epoll.o time.o sockets.o epollselector.o
	$(LINK) $@ $^			$(LIBS)


%.o: %.cc
	$(CXX) $<

-include $(SRC:%.cc=%.d)

