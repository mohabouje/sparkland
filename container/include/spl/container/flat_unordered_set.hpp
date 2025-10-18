#pragma once

#include <boost/unordered/unordered_flat_set.hpp>
#include <functional>

namespace spl::container {

    template <typename Key,                                //
              typename Hash  = std::hash<Key>,             //
              typename KeyEqual = std::equal_to<Key>,      //
              typename Allocator = std::allocator<Key>>
    using flat_unordered_set = boost::unordered_flat_set<Key, Hash, KeyEqual, Allocator>;

} // namespace spl::container
