MYCC	= clang
MYCC	= g++

CXX_OPT	= -g
CXX_OPT	= -O9
#CXX_OPT	=

CXX	= $(MYCC) -std=c++11 -Wall  -Wconversion 	\
		-D_FILE_OFFSET_BITS=64	\
		-MMD -MP		\
		$(CXX_OPT)		\
		-c

LINK	= $(MYCC) -o
LIBS	= -lstdc++

#-ljemalloc
#-fpack-struct

SRC	= $(wildcard *.cc)


TARGETS	=	test


all: $(TARGETS)


clean:
	rm -f *.o *.d *.gch		\
			$(TARGETS)

test: test.o mysocket.o asyncpollselector.o
	$(LINK) $@ $^			$(LIBS)


%.o: %.cc
	$(CXX) $<

-include $(SRC:%.cc=%.d)

