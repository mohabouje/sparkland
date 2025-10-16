#include "spl/network/common/result.hpp"

#include <gtest/gtest.h>

TEST(NetworkTest, SimpleErrorFromBoostAsio) {
    auto const network_error = spl::network::network_error{boost::asio::stream_errc::eof};
    ASSERT_EQ(network_error.value(), boost::asio::stream_errc::eof);
    ASSERT_EQ(std::string_view(network_error.message().data(), network_error.message().size()),
              std::string_view("End of file"));
}

TEST(NetworkTest, ConvertToGenericError) {
    auto const network_error = spl::network::result<int>(spl::network::network_error{boost::asio::stream_errc::eof});
    auto const error         = spl::outcome::result<int>{network_error};
    EXPECT_EQ(error.error().value().what(), "End of file");
}