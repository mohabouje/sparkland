#define SPL_CONTRACTS_ENABLED
#include "spl/result/contract.hpp"

#include <gtest/gtest.h>

TEST(ContractTest, ExpectFailureUnderViolation) {
    ASSERT_DEATH({ spl::ensures(false, "Error on line {} of {}", __LINE__, __FILE__); }, "");
}

TEST(ContractTest, DoesNotExpectFailureUnderNoViolation) {
    spl::ensures(true, "Error on line {} of {}", __LINE__, __FILE__);
}
