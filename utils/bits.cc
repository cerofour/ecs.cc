#include "bits.hpp"

namespace utils {
	namespace bits {
		u64 setbit(u64 bitno, u64& x) {
			x |= (1 << bitno);
			return x;
		}
		bool isbiton(u64 bitno, u64 x) {
			return x & (1 << bitno);
		}
		bool checkmask(u64 x, u64 mask) {
			return (x & mask) == mask;
		}
	};
};
