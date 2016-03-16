#include "sockets.h"
#include "asyncloop.h"

#include "epollselector.h"

using MySelector = net::EPollSelector;

#include "test_implementation.cc"

