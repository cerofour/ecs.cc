#include <vector>

#include "types.hpp"

namespace utils {
    namespace numeric {
        u64 set_bits(const std::vector<u64>& bits) {
            u64 result = 0;
            for (const auto x : bits)
                result |= (1 << x);
            return result;
        }
    };
};
