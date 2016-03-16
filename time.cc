#include "time.h"

#include <chrono>

namespace net{



std::uint32_t now(){
	const auto now = std::chrono::system_clock::now().time_since_epoch();

	const auto sec = std::chrono::duration_cast<std::chrono::seconds>(now);

	return (uint32_t) sec.count();
}



} // namespace

