#ifndef _ASYNC_LOOP_DEFS_H
#define _ASYNC_LOOP_DEFS_H

namespace AsyncLoopDefs{
	enum class WaitStatus{ OK, NONE, ERROR };
	enum class FDStatus{ NONE, READ, WRITE, ERROR };
};

#endif

