#pragma once

#include <chrono>

namespace spl::concepts {
    namespace internal {

        template <typename T>
        struct is_duration : std::false_type {};

        template <typename RepT, typename PeriodT>
        struct is_duration<std::chrono::duration<RepT, PeriodT>> : std::true_type {};

    } // namespace internal

    template <typename T>
    concept duration = internal::is_duration<T>::value;

} // namespace spl::concepts