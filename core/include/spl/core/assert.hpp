#pragma once

#ifdef SPL_HARDENED
#    include <stacktrace>
#    define SPL_ASSERT(expr)                                                                                           \
        do {                                                                                                           \
            if (!(expr)) [[unlikely]] {                                                                                \
                std::fprintf(stderr, "Assertion failed: %s\n", #expr);                                                 \
                std::fprintf(stderr, "Stack trace: %s\n", std::to_string(std::stacktrace::current()).c_str());         \
                std::abort();                                                                                          \
            }                                                                                                          \
        } while (false)

#    define SPL_ASSERT_MSG(expr, msg)                                                                                  \
        do {                                                                                                           \
            if (!(expr)) [[unlikely]] {                                                                                \
                std::fprintf(stderr, "Assertion failed: %s\n (%s)\n", #expr, msg);                                     \
                std::fprintf(stderr, "Stack trace: %s\n", std::to_string(std::stacktrace::current()).c_str());         \
                std::abort();                                                                                          \
            }                                                                                                          \
        } while (false)
#else
#    include <cassert>
#    define SPL_ASSERT(expr)          assert(expr)
#    define SPL_ASSERT_MSG(expr, msg) assert(expr&& msg)
#endif
