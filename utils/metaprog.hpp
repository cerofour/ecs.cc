#include <type_traits>

#include "types.hpp"

namespace utils {
    namespace metaprog {
        template <typename...>
        struct is_one_of {
            static constexpr bool value = false;
        };

        template <typename F, typename S, typename... T>
        struct is_one_of<F, S, T...> {
            static constexpr bool value =
                std::is_same<F, S>::value || is_one_of<F, T...>::value;
        };

        template<
            typename First,
            typename Second,
            typename ...Rest>
        constexpr bool only_unique_types() {
            if constexpr (sizeof...(Rest) == 0) {
                return !std::is_same<First, Second>();
            } else {
                if (std::is_same<First, Second>() || is_one_of<First, Rest...>::value)
                    return false;
                else
                    return only_unique_types<Second, Rest...>();
            }
        }

        template<
            typename Target,
            typename First,
            typename ...Rest>
        constexpr u64 index() {
            static_assert(std::is_same<Target, First>() ||
                    is_one_of<Target, Rest...>::value,
                    "Target is not in the type list");

            if constexpr (std::is_same<Target, First>())
                return 0;
            else
                return 1 + index<Target, Rest...>();
        }
    };
};
