#pragma once

#include <boost/container/static_vector.hpp>

namespace spl { inline namespace container {

    template <class T, std::size_t N>
    using static_vector = boost::container::static_vector<T, N>;

}} // namespace spl::container
