#ifndef _NET_STATUS_H
#define _NET_STATUS_H

namespace net{

enum class WaitStatus{ OK, NONE, ERROR };
enum class FDStatus{ NONE, READ, WRITE, ERROR, STOP };

} // namespace

#endif

