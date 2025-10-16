#pragma once

#include "spl/core/assert.hpp"
#include "spl/concepts/types.hpp"

#include <cstddef>
#include <iterator>

namespace spl::types {

    template <concepts::byte_like TypeT>
    struct cursor {
        using value_type      = TypeT;
        using pointer         = value_type*;
        using size_type       = std::size_t;
        using difference_type = std::ptrdiff_t;

        constexpr cursor() noexcept                                 = default;
        constexpr cursor(cursor const&) noexcept                    = default;
        constexpr cursor(cursor&&) noexcept                         = default;
        constexpr auto operator=(cursor const&) noexcept -> cursor& = default;
        constexpr auto operator=(cursor&&) noexcept -> cursor&      = default;

        constexpr cursor(pointer ptr, pointer end) noexcept : ptr_(ptr), end_(end) {
            SPL_ASSERT_MSG(ptr <= end, "cursor out of bounds");
        }

        constexpr cursor(pointer ptr, size_type size) noexcept : cursor(ptr, ptr + size) {}

        constexpr cursor(std::span<value_type const> span) noexcept :
            ptr_(std::data(span)), end_(std::data(span) + std::size(span)) {}

        template <typename ElementT, bool MoveCursorV = true>
        [[nodiscard]] constexpr auto reinterpret() noexcept -> ElementT const* {
            SPL_ASSERT_MSG(ptr_ + sizeof(ElementT) <= end_, "cursor out of bounds");
            SPL_ASSERT_MSG(reinterpret_cast<std::uintptr_t>(ptr_) % alignof(ElementT) == 0, "misaligned access");
            auto const* ptr = reinterpret_cast<ElementT const*>(ptr_);
            if constexpr (MoveCursorV) {
                ptr_ += sizeof(ElementT);
            }
            return ptr;
        }

        constexpr auto operator+=(size_type size) noexcept -> cursor& {
            SPL_ASSERT_MSG(ptr_ + size <= end_, "cursor out of bounds");
            ptr_ += size;
            return *this;
        }

        constexpr auto operator++() noexcept -> cursor& {
            SPL_ASSERT_MSG(ptr_ + sizeof(value_type) <= end_, "cursor out of bounds");
            ptr_ += sizeof(value_type);
            return *this;
        }

        [[nodiscard]] constexpr auto operator+(size_type size) noexcept -> cursor {
            SPL_ASSERT_MSG(ptr_ + size <= end_, "cursor out of bounds");
            return cursor{ptr_ + size, end_};
        }

        [[nodiscard]] constexpr auto operator++(int) noexcept -> cursor {
            auto old = *this;
            ++*this;
            return old;
        }

        [[nodiscard]] constexpr auto remaining() const noexcept -> difference_type {
            return end_ - ptr_;
        }

        [[nodiscard]] constexpr auto size() const noexcept -> difference_type {
            return end_ - ptr_;
        }

        [[nodiscard]] constexpr auto empty() const noexcept -> bool {
            return ptr_ >= end_;
        }

        [[nodiscard]] constexpr auto data() const noexcept -> pointer {
            return ptr_;
        }

        [[nodiscard]] constexpr auto end() const noexcept -> pointer {
            return end_;
        }

        constexpr auto advance(size_type size) noexcept -> void {
            SPL_ASSERT_MSG(ptr_ + size <= end_, "cursor out of bounds");
            ptr_ += size;
        }

        [[nodiscard]] constexpr auto subcursor(size_type size) noexcept -> cursor {
            SPL_ASSERT_MSG(ptr_ + size <= end_, "cursor out of bounds");
            return cursor{ptr_, ptr_ + size};
        }

        [[nodiscard]] constexpr auto consume(size_type size) noexcept -> cursor {
            SPL_ASSERT_MSG(ptr_ + size <= end_, "cursor out of bounds");
            auto const copy = cursor{ptr_, ptr_ + size};
            ptr_ += size;
            return copy;
        }

        [[nodiscard]] constexpr auto span() noexcept -> std::span<value_type const> {
            return std::span<value_type const>{ptr_, end_};
        }

        constexpr auto clear() noexcept -> void {
            ptr_ = end_;
        }

    private:
        pointer ptr_;
        pointer end_;
    };

} // namespace spl::types
