#ifndef BOOST_TEXT_STRING_BUILDER_HPP
#define BOOST_TEXT_STRING_BUILDER_HPP

#include <boost/text/string.hpp>

#include <numeric>


namespace boost { namespace text {

    namespace detail {
        template<int N>
        struct static_string_builder_impl;

        template<>
        struct static_string_builder_impl<0>
        {
            int size() const noexcept { return 0; }
            void copy(char *) const noexcept {}
        };

        template<int N>
        struct static_string_builder_impl
        {
            int size() const noexcept { return view_.size() + prev_.size(); }

            void copy(char * last) const noexcept
            {
                last = std::copy_backward(view_.begin(), view_.end(), last);
                prev_.copy(last);
            }

            string to_string() const
            {
                string retval;
                retval.resize(size(), 0);
                copy(retval.end());
                return retval;
            }

            static_string_builder_impl<N - 1> prev_;
            string_view view_;
        };
    }

    /** A type for efficiently building strings from string_views, with only a
        single allocation.  This type is constexpr-friendly in C++14 and
        later. */
    struct static_string_builder
    {
        constexpr static_string_builder() {}
        explicit constexpr static_string_builder(string_view sv) :
            impl_{{}, {sv}}
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
        return {prev.impl_, sv};
    }

    namespace detail {
        /** Returns a type that represents the concatenation of prev and
            sv. */
        template<int N>
        constexpr static_string_builder_impl<N + 1>
        operator+(static_string_builder_impl<N> prev, string_view sv) noexcept
        {
            return {prev, sv};
        }
    }

    /** A type for efficiently building strings from string_views. */
    struct string_builder
    {
        string_builder() : data_(), size_(0), cap_(0) {}
        explicit string_builder(string_view sv) :
            data_(new char[sv.size()]),
            size_(sv.size()),
            cap_(sv.size())
        {
            std::copy(sv.begin(), sv.end(), data_.get());
        }

        string to_string() noexcept
        {
            return string(std::move(data_), size_, cap_);
        }

        string_builder & operator+=(string_view sv)
        {
            auto const new_size = size_ + sv.size();
            if (cap_ < new_size) {
                int const new_cap = cap_ < 32 ? 32 : cap_ / 2 * 3;
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

}}

#endif
