
#include "spl/math/rounding.hpp"

#include <algorithm>
#include <array>
#include <type_traits>
#include <utility>
#include <vector>

#include <gtest/gtest.h>

using namespace spl::math;
using namespace spl::math::detail;

namespace {

    // Workaround for https://llvm.org/bugs/show_bug.cgi?id=16404,
    // issues with __int128 multiplication and UBSAN
    template <typename T>
    T mul(T lhs, T rhs) {
        if (rhs < 0) {
            rhs = -rhs;
            lhs = -lhs;
        }
        T accum = 0;
        while (rhs != 0) {
            if ((rhs & 1) != 0) {
                accum += lhs;
            }
            lhs += lhs;
            rhs >>= 1;
        }
        return accum;
    }

    template <typename T, typename B>
    T referenceDiv_floor(T numer, T denom) {
        // rv = largest integral value <= numer / denom
        B n = numer;
        B d = denom;
        if (d < 0) {
            d = -d;
            n = -n;
        }
        B r = n / d;
        while (mul(r, d) > n) {
            --r;
        }
        while (mul(r + 1, d) <= n) {
            ++r;
        }
        T rv = static_cast<T>(r);
        assert(static_cast<B>(rv) == r);
        return rv;
    }

    template <typename T, typename B>
    T reference_div_ceil(T numer, T denom) {
        // rv = smallest integral value >= numer / denom
        B n = numer;
        B d = denom;
        if (d < 0) {
            d = -d;
            n = -n;
        }
        B r = n / d;
        while (mul(r, d) < n) {
            ++r;
        }
        while (mul(r - 1, d) >= n) {
            --r;
        }
        T rv = static_cast<T>(r);
        assert(static_cast<B>(rv) == r);
        return rv;
    }

    template <typename T, typename B>
    T reference_div_round_away(T numer, T denom) {
        if ((numer < 0) != (denom < 0)) {
            return referenceDiv_floor<T, B>(numer, denom);
        } else {
            return reference_div_ceil<T, B>(numer, denom);
        }
    }

    template <typename T>
    std::vector<T> cornerValues() {
        std::vector<T> rv;
        for (T i = 1; i < 24; ++i) {
            rv.push_back(i);
            rv.push_back(T(std::numeric_limits<T>::max() / i));
            rv.push_back(T(std::numeric_limits<T>::max() - i));
            rv.push_back(T(std::numeric_limits<T>::max() / T(2) - i));
            if (std::is_signed<T>::value) {
                rv.push_back(-i);
                rv.push_back(T(std::numeric_limits<T>::min() / i));
                rv.push_back(T(std::numeric_limits<T>::min() + i));
                rv.push_back(T(std::numeric_limits<T>::min() / T(2) + i));
            }
        }
        return rv;
    }

    template <typename A, typename B, typename C>
    void run_div_tests() {
        using T      = decltype(static_cast<A>(1) / static_cast<B>(1));
        auto numbers = cornerValues<A>();
        numbers.push_back(0);
        auto denoms = cornerValues<B>();
        for (A n : numbers) {
            for (B d : denoms) {
                if (std::is_signed<T>::value && n == std::numeric_limits<T>::min() /* && d == static_cast<T>(-1) */) {
                    // n / d overflows in two's complement
                    continue;
                }
                EXPECT_EQ(div_ceil(n, d), (reference_div_ceil<T, C>(n, d))) << n << "/" << d;
                EXPECT_EQ(div_floor(n, d), (referenceDiv_floor<T, C>(n, d))) << n << "/" << d;
                EXPECT_EQ(div_trunc(n, d), n / d) << n << "/" << d;
                EXPECT_EQ(div_round_away(n, d), (reference_div_round_away<T, C>(n, d))) << n << "/" << d;
                T nn = n;
                T dd = d;
                EXPECT_EQ(div_ceil_branchless(nn, dd), div_ceil_branchful(nn, dd));
                EXPECT_EQ(div_floor_branchless(nn, dd), div_floor_branchful(nn, dd));
                EXPECT_EQ(div_round_away_branchless(nn, dd), div_round_away_branchful(nn, dd));
            }
        }
    }
} // namespace

TEST(RoundingTest, divTestInt8) {
    run_div_tests<int8_t, int8_t, int64_t>();
    run_div_tests<int8_t, uint8_t, int64_t>();
    run_div_tests<int8_t, int16_t, int64_t>();
    run_div_tests<int8_t, uint16_t, int64_t>();
    run_div_tests<int8_t, int32_t, int64_t>();
    run_div_tests<int8_t, uint32_t, int64_t>();
    run_div_tests<int8_t, int64_t, __int128>();
    run_div_tests<int8_t, uint64_t, __int128>();
}

TEST(RoundingTest, divTestInt16) {
    run_div_tests<int16_t, int8_t, int64_t>();
    run_div_tests<int16_t, uint8_t, int64_t>();
    run_div_tests<int16_t, int16_t, int64_t>();
    run_div_tests<int16_t, uint16_t, int64_t>();
    run_div_tests<int16_t, int32_t, int64_t>();
    run_div_tests<int16_t, uint32_t, int64_t>();
    run_div_tests<int16_t, int64_t, __int128>();
    run_div_tests<int16_t, uint64_t, __int128>();
}
TEST(RoundingTest, divTestInt32) {
    run_div_tests<int32_t, int8_t, int64_t>();
    run_div_tests<int32_t, uint8_t, int64_t>();
    run_div_tests<int32_t, int16_t, int64_t>();
    run_div_tests<int32_t, uint16_t, int64_t>();
    run_div_tests<int32_t, int32_t, int64_t>();
    run_div_tests<int32_t, uint32_t, int64_t>();
    run_div_tests<int32_t, int64_t, __int128>();
    run_div_tests<int32_t, uint64_t, __int128>();
}

TEST(RoundingTest, divTestInt64) {
    run_div_tests<int64_t, int8_t, __int128>();
    run_div_tests<int64_t, uint8_t, __int128>();
    run_div_tests<int64_t, int16_t, __int128>();
    run_div_tests<int64_t, uint16_t, __int128>();
    run_div_tests<int64_t, int32_t, __int128>();
    run_div_tests<int64_t, uint32_t, __int128>();
    run_div_tests<int64_t, int64_t, __int128>();
    run_div_tests<int64_t, uint64_t, __int128>();
}

TEST(RoundingTest, divTestUint8) {
    run_div_tests<uint8_t, int8_t, int64_t>();
    run_div_tests<uint8_t, uint8_t, int64_t>();
    run_div_tests<uint8_t, int16_t, int64_t>();
    run_div_tests<uint8_t, uint16_t, int64_t>();
    run_div_tests<uint8_t, int32_t, int64_t>();
    run_div_tests<uint8_t, uint32_t, int64_t>();
    run_div_tests<uint8_t, int64_t, __int128>();
    run_div_tests<uint8_t, uint64_t, __int128>();
}

TEST(RoundingTest, divTestUint16) {
    run_div_tests<uint16_t, int8_t, int64_t>();
    run_div_tests<uint16_t, uint8_t, int64_t>();
    run_div_tests<uint16_t, int16_t, int64_t>();
    run_div_tests<uint16_t, uint16_t, int64_t>();
    run_div_tests<uint16_t, int32_t, int64_t>();
    run_div_tests<uint16_t, uint32_t, int64_t>();
    run_div_tests<uint16_t, int64_t, __int128>();
    run_div_tests<uint16_t, uint64_t, __int128>();
}

TEST(RoundingTest, divTestUint32) {
    run_div_tests<uint32_t, int8_t, int64_t>();
    run_div_tests<uint32_t, uint8_t, int64_t>();
    run_div_tests<uint32_t, int16_t, int64_t>();
    run_div_tests<uint32_t, uint16_t, int64_t>();
    run_div_tests<uint32_t, int32_t, int64_t>();
    run_div_tests<uint32_t, uint32_t, int64_t>();
    run_div_tests<uint32_t, int64_t, __int128>();
    run_div_tests<uint32_t, uint64_t, __int128>();
}

TEST(RoundingTest, divTestUint64) {
    run_div_tests<uint64_t, int8_t, __int128>();
    run_div_tests<uint64_t, uint8_t, __int128>();
    run_div_tests<uint64_t, int16_t, __int128>();
    run_div_tests<uint64_t, uint16_t, __int128>();
    run_div_tests<uint64_t, int32_t, __int128>();
    run_div_tests<uint64_t, uint32_t, __int128>();
    run_div_tests<uint64_t, int64_t, __int128>();
    run_div_tests<uint64_t, uint64_t, __int128>();
}
