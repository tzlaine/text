// Copyright (C) 2020 T. Zachary Laine
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
#ifndef BOOST_TEXT_STRING_BUILDER_HPP
#define BOOST_TEXT_STRING_BUILDER_HPP

#include <boost/text/string.hpp>

#include <numeric>


namespace boost { namespace text { inline namespace v1 {

    namespace detail {
        template<int N>
        struct static_string_builder_impl;

        template<>
        struct static_string_builder_impl<0>
        {
            constexpr static_string_builder_impl() : size_(0) {}
            void copy(char *) const noexcept {}
            int size_;
        };

        template<int N>
        struct static_string_builder_impl
        {
            void copy(char * last) const noexcept
            {
                last = std::copy_backward(view_.begin(), view_.end(), last);
                prev_.copy(last);
            }

            string to_string() const
            {
                string retval;
                retval.resize(size_, 0);
                copy(retval.end());
                return retval;
            }

            static_string_builder_impl<N - 1> prev_;
            string_view view_;
            int size_;
        };
    }

    /** A type for efficiently building strings from string_views, with only a
        single allocation.  This type is constexpr-friendly in C++14 and
        later. */
    struct static_string_builder
    {
        constexpr static_string_builder() : impl_{{}, {}, 0} {}
        explicit constexpr static_string_builder(string_view sv) :
            impl_{{}, {sv}, sv.size()}
        {}

        string to_string() const noexcept { return impl_.to_string(); }

    private:
        detail::static_string_builder_impl<1> impl_;

        friend constexpr detail::static_string_builder_impl<2>
        operator+(static_string_builder b, string_view sv) noexcept;
    };

    /** Returns a type that represents the concatenation of prev and sv. */
    inline constexpr detail::static_string_builder_impl<2>
    operator+(static_string_builder prev, string_view sv) noexcept
    {
        return {prev.impl_, sv, prev.impl_.size_ + sv.size()};
    }

    namespace detail {
        /** Returns a type that represents the concatenation of prev and
            sv. */
        template<int N>
        constexpr static_string_builder_impl<N + 1>
        operator+(static_string_builder_impl<N> prev, string_view sv) noexcept
        {
            return {prev, sv, prev.size_ + sv.size()};
        }
    }

    /** A type for efficiently building strings from string_views. */
    struct string_builder
    {
        string_builder() : data_(), size_(0), cap_(0) {}
        explicit string_builder(string_view sv) :
            data_(new char[sv.size() + 1]),
            size_(sv.size()),
            cap_(sv.size() + 1)
        {
            std::copy(sv.begin(), sv.end(), data_.get());
        }

        string to_string() noexcept
        {
            if (data_) {
                data_[size_] = 0;
                return string(std::move(data_), size_, cap_);
            } else {
                return string();
            }
        }

        string_builder & operator+=(string_view sv)
        {
            auto const min_cap = size_ + sv.size() + 1;
            if (cap_ < min_cap) {
                int const new_cap = (std::max)(min_cap, 32) / 2 * 3;
                std::unique_ptr<char[]> new_data(new char[new_cap]);
                std::copy(data_.get(), data_.get() + size_, new_data.get());
                std::swap(data_, new_data);
                cap_ = new_cap;
            }

            std::copy(sv.begin(), sv.end(), data_.get() + size_);
            size_ += sv.size();

            return *this;
        }

    private:
        std::unique_ptr<char[]> data_;
        int size_;
        int cap_;
    };

}}}

#endif
