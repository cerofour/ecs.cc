#include "types.hpp"

namespace utils {
	namespace bits {
		u64 setbit(u64 bitno, u64& x);
		bool isbiton(u64 bitno, u64 x);
		bool checkmask(u64 x, u64 mask);
	};
};
