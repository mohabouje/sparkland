#define SPL_CONTRACTS_DISABLED
#include "spl/result/contract.hpp"

#include <gtest/gtest.h>

auto please_never_call() -> bool {
    throw std::runtime_error("This object should have never been created if the contracts are ignored");
    return false;
}

TEST(ContractDisabledTest, DoesNotEvenTestTheContractIfItsDisabledWithMacros) {
    // SPL_ENSURES(please_never_call(), "simple example of a potential violation");
}

TEST(ContractDisabledTest, DoesTestTheContractIfItsDisabledButIfDoesNotFail) {
    EXPECT_ANY_THROW(spl::ensures(please_never_call(), "simple example of a potential violation"));
}