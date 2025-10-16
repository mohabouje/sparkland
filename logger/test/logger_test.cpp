#include "spl/logger/logger.hpp"

#include <gtest/gtest.h>

TEST(LoggerTest, PrintDifferentLevels) {
    spl::logger::trace("this is a trace message {}", "test");
    spl::logger::debug("this is a debug message {}", "test");
    spl::logger::warn("this is a warn message {}", "test");
    spl::logger::info("this is a info message {}", "test");
    spl::logger::error("this is a error message {}", "test");
    spl::logger::fatal("this is a fatal message {}", "test");
}
