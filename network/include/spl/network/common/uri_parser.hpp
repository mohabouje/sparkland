#pragma once

#include "spl/result/result.hpp"

#include <boost/url.hpp>

namespace spl::network::common {

    [[nodiscard]] inline auto host(std::string_view uri) noexcept -> spl::result<std::string> {
        auto const result = boost::urls::parse_uri(uri);
        if (!result) {
            return failure("{}", result.error().message().data());
        }
        return std::string(result->encoded_host());
    }

    [[nodiscard]] inline auto path(std::string_view uri) noexcept -> spl::result<std::string> {
        auto const result = boost::urls::parse_uri(uri);
        if (!result) {
            return failure("{}", result.error().message().data());
        }
        return std::string(result->encoded_path());
    }

    [[nodiscard]] inline auto query(std::string_view uri) noexcept -> spl::result<std::string> {
        auto const result = boost::urls::parse_uri(uri);
        if (!result) {
            return failure("{}", result.error().message().data());
        }
        return std::string(result->encoded_query());
    }

} // namespace spl::network::common
