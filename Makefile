MYCC		= clang
MYCC		= g++

# ======================================================

EXTRA_CLEAN	:=
EXTRA_INCL	:=

# ======================================================

CF_DEPS		= -MMD -MP
CF_INCL		= -Iinclude $(EXTRA_INCL)
CF_OPTIM	=
CF_WARN		= -Wall -Wconversion -Wpedantic

CF_MISC		=

CF_ALL		= -std=c++11	\
			$(CF_DEPS)	\
			$(CF_INCL)	\
			$(CF_OPTIM)	\
			$(CF_WARN)	\
			$(CF_MISC)

CXX		= $(MYCC) $(CF_ALL)

# ======================================================

LD_ALL		=
LL_ALL		= -lstdc++

LINK		= $(MYCC) $(LD_ALL) -o $@ $^ $(LL_ALL)

SRC		= $(wildcard *.cc)

# ======================================================

A		= bin/
O		= obj/

# ======================================================
# ======================================================
# ======================================================

SUBDIRS = net net/selector net/protocol net/worker net/test

include $(addsuffix /Makefile.dir, $(SUBDIRS))

clean:
	rm -f \
		$(O)*.o		\
		$(O)*.d		\
		$(EXTRA_CLEAN)

