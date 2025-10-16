#include "spl/components/scheduler/scheduler.hpp"

#include <gtest/gtest.h>

TEST(ComponentsTimerSchedulerTest, SingleshotTimer) {
    spl::components::scheduler::scheduler scheduler;
    auto& timer = scheduler.oneshot(std::chrono::seconds(1), []() { return spl::success(); });
    EXPECT_EQ(scheduler.size(), 1);
    EXPECT_EQ(scheduler.contains(timer.id()), true);
}

TEST(ComponentsTimerSchedulerTest, PeriodicTimer) {
    spl::components::scheduler::scheduler scheduler;
    auto& timer = scheduler.periodic(std::chrono::seconds(1), []() { return spl::success(); });
    EXPECT_EQ(scheduler.size(), 1);
    EXPECT_EQ(scheduler.contains(timer.id()), true);
}
