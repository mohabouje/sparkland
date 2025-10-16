#pragma once
// C++20 Compile-time XXH32()
//
// g++-10 (10.2.0)          g++-10 -std=c++20 -DINCLUDE_Xxxhash ./cxx20_ct_xxxhash.cpp
// Clang (10.0.0)           clang  -std=c++20 -DINCLUDE_Xxxhash ./cxx20_ct_xxxhash.cpp
// Visual C++ 2019 (19.28)  cl /std:c++latest /DINCLUDE_Xxxhash .\cxx20_ct_xxxhash.cpp
//
// Result at Compiler Explorer
// https://godbolt.org/z/bGv7Mh

#include <cstdint>
#include <bit>
#include <string_view>

namespace spl::concepts::xxhash::detail {
    constexpr auto rotl(uint32_t v, int x) -> uint32_t {
        return std::rotl(v, x);
    }

    constexpr auto read_u8(char const* input, int pos) -> uint8_t {
        return static_cast<uint8_t>(input[pos]);
    }

    constexpr auto read_u32le(char const* input, int pos) -> uint32_t {
        uint32_t const b0 = read_u8(input, pos + 0);
        uint32_t const b1 = read_u8(input, pos + 1);
        uint32_t const b2 = read_u8(input, pos + 2);
        uint32_t const b3 = read_u8(input, pos + 3);
        return b0 | (b1 << 8) | (b2 << 16) | (b3 << 24);
    }
} // namespace spl::concepts::xxhash::detail

namespace spl::concepts::xxhash::detail::xxh32 {
    constexpr uint32_t prime32_1 = 0x9E3779B1U;
    constexpr uint32_t prime32_2 = 0x85EBCA77U;
    constexpr uint32_t prime32_3 = 0xC2B2AE3DU;
    constexpr uint32_t prime32_4 = 0x27D4EB2FU;
    constexpr uint32_t prime32_5 = 0x165667B1U;

    constexpr auto xxh32_avalanche(uint32_t h32) -> uint32_t {
        h32 ^= h32 >> 15;
        h32 *= prime32_2;
        h32 ^= h32 >> 13;
        h32 *= prime32_3;
        h32 ^= h32 >> 16;
        return h32;
    }

    constexpr auto xxh32_finalize(char const* input, int length, int pos, uint32_t h32) -> uint32_t {
        // XXH_PROCESS4
        while ((length - pos) >= 4) {
            h32 += read_u32le(input, pos) * prime32_3;
            h32 = rotl(h32, 17) * prime32_4;
            pos += 4;
        }
        // XXH_PROCESS1
        while ((length - pos) > 0) {
            h32 += read_u8(input, pos) * prime32_5;
            h32 = rotl(h32, 11) * prime32_1;
            pos += 1;
        }
        return h32;
    }

    constexpr auto xxh32_digest(char const* input, int length, int pos, uint32_t v1, uint32_t v2, uint32_t v3,
                                uint32_t v4) -> uint32_t {
        uint32_t h32 = 0;
        if (length >= 16) {
            h32 = rotl(v1, 1) + rotl(v2, 7) + rotl(v3, 12) + rotl(v4, 18);
        } else {
            h32 = v3 + prime32_5;
        }
        h32 += length;
        h32 = xxh32_finalize(input, length, pos, h32);
        return xxh32_avalanche(h32);
    }

    constexpr auto xxh32_round(uint32_t acc, char const* input, int pos) -> uint32_t {
        uint32_t const d = read_u32le(input, pos);
        acc += d * prime32_2;
        acc = rotl(acc, 13) * prime32_1;
        return acc;
    }

    constexpr auto xxh32(char const* input, int length, uint32_t seed) -> uint32_t {
        uint32_t v1 = seed + prime32_1 + prime32_2;
        uint32_t v2 = seed + prime32_2;
        uint32_t v3 = seed;
        uint32_t v4 = seed - prime32_1;
        int pos     = 0;
        while (pos + 16 <= length) {
            v1 = xxh32_round(v1, input, pos + 0 * 4);
            v2 = xxh32_round(v2, input, pos + 1 * 4);
            v3 = xxh32_round(v3, input, pos + 2 * 4);
            v4 = xxh32_round(v4, input, pos + 3 * 4);
            pos += 16;
        }
        return xxh32_digest(input, length, pos, v1, v2, v3, v4);
    }
} // namespace spl::concepts::xxhash::detail::xxh32

namespace spl::concepts::xxhash {
    [[nodiscard]] constexpr auto xxh32(char const* input, std::size_t length) -> uint32_t {
        return detail::xxh32::xxh32(input, static_cast<int>(length), 0);
    }

    [[nodiscard]] constexpr auto xxh32(std::string_view input) -> uint32_t {
        return xxh32(std::data(input), std::size(input));
    }
} // namespace spl::concepts::xxhash