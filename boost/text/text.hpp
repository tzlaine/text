#ifndef BOOST_TEXT_TEXT_HPP
#define BOOST_TEXT_TEXT_HPP

#include <iterator>
#include <memory>

#include <assert.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>


namespace boost { namespace text {

    struct text;
    struct text_view;

    namespace detail {

        inline constexpr std::size_t strlen (char const * c_str) noexcept
        {
            std::size_t retval = 0;
            while (c_str) {
                retval += 1;
                ++c_str;
            }
            return retval;
        }

        inline constexpr char const * strrchr (
            char const * first,
            char const * last,
            char c
        ) noexcept {
            while (first != last) {
                if (*--last == c)
                    return last;
            }
            return nullptr;
        }

        template <typename T>
        constexpr T min_ (T rhs, T lhs) noexcept
        { return rhs < lhs ? rhs : lhs; }

        struct reverse_char_iterator
        {
            using value_type = char;
            using difference_type = int;
            using pointer = char *;
            using reference = char &;
            using iterator_category = std::random_access_iterator_tag;

            constexpr reverse_char_iterator () noexcept :
                ptr_ (nullptr)
            {}
            explicit constexpr reverse_char_iterator (char * ptr) noexcept :
                ptr_ (ptr)
            {}

            constexpr char * base () const noexcept
            { return ptr_ + 1; }

            constexpr reference operator* () const noexcept
            { return *ptr_; }
            constexpr pointer operator->() const noexcept
            { return ptr_; }

            constexpr value_type operator[] (difference_type n) const noexcept
            { return ptr_[n]; }

            constexpr reverse_char_iterator operator++ () noexcept
            {
                ++ptr_;
                return *this;
            }
            constexpr reverse_char_iterator operator++ (int) noexcept
            {
                reverse_char_iterator retval = *this;
                ++ptr_;
                return retval;
            }
            constexpr reverse_char_iterator operator+= (int n) noexcept
            {
                ptr_ += n;
                return *this;
            }

            constexpr reverse_char_iterator operator-- () noexcept
            {
                --ptr_;
                return *this;
            }
            constexpr reverse_char_iterator operator-- (int) noexcept
            {
                reverse_char_iterator retval = *this;
                --ptr_;
                return retval;
            }
            constexpr reverse_char_iterator operator-= (int n) noexcept
            {
                ptr_ += n;
                return *this;
            }

            // TODO: operator<=> () const
            friend constexpr bool operator== (reverse_char_iterator lhs, reverse_char_iterator rhs) noexcept
            { return lhs.ptr_ == rhs.ptr_; }
            friend constexpr bool operator!= (reverse_char_iterator lhs, reverse_char_iterator rhs) noexcept
            { return lhs.ptr_ != rhs.ptr_; }
            friend constexpr bool operator< (reverse_char_iterator lhs, reverse_char_iterator rhs) noexcept
            { return lhs.ptr_ < rhs.ptr_; }
            friend constexpr bool operator<= (reverse_char_iterator lhs, reverse_char_iterator rhs) noexcept
            { return lhs.ptr_ <= rhs.ptr_; }
            friend constexpr bool operator> (reverse_char_iterator lhs, reverse_char_iterator rhs) noexcept
            { return lhs.ptr_ > rhs.ptr_; }
            friend constexpr bool operator>= (reverse_char_iterator lhs, reverse_char_iterator rhs) noexcept
            { return lhs.ptr_ >= rhs.ptr_; }

            friend constexpr reverse_char_iterator operator+ (reverse_char_iterator lhs, int rhs) noexcept
            { return lhs += rhs; }
            friend constexpr reverse_char_iterator operator+ (int lhs, reverse_char_iterator rhs) noexcept
            { return rhs += lhs; }
            friend constexpr reverse_char_iterator operator- (reverse_char_iterator lhs, int rhs) noexcept
            { return lhs -= rhs; }
            friend constexpr reverse_char_iterator operator- (int lhs, reverse_char_iterator rhs) noexcept
            { return rhs -= lhs; }
            friend constexpr int operator- (reverse_char_iterator lhs, reverse_char_iterator rhs) noexcept
            { return lhs.ptr_ - rhs.ptr_; }

        private:
            char * ptr_;
        };

        struct const_reverse_char_iterator
        {
            using value_type = char const;
            using difference_type = int;
            using pointer = char const *;
            using reference = char const;
            using iterator_category = std::random_access_iterator_tag;

            constexpr const_reverse_char_iterator () noexcept :
                ptr_ (nullptr)
            {}
            explicit constexpr const_reverse_char_iterator (char const * ptr) noexcept :
                ptr_ (ptr)
            {}

            constexpr char const * base () const noexcept
            { return ptr_ + 1; }

            constexpr reference operator* () const noexcept
            { return *ptr_; }
            constexpr pointer operator->() const noexcept
            { return ptr_; }

            constexpr value_type operator[] (difference_type n) const noexcept
            { return ptr_[n]; }

            constexpr const_reverse_char_iterator operator++ () noexcept
            {
                ++ptr_;
                return *this;
            }
            constexpr const_reverse_char_iterator operator++ (int) noexcept
            {
                const_reverse_char_iterator retval = *this;
                ++ptr_;
                return retval;
            }
            constexpr const_reverse_char_iterator operator+= (int n) noexcept
            {
                ptr_ += n;
                return *this;
            }

            constexpr const_reverse_char_iterator operator-- () noexcept
            {
                --ptr_;
                return *this;
            }
            constexpr const_reverse_char_iterator operator-- (int) noexcept
            {
                const_reverse_char_iterator retval = *this;
                --ptr_;
                return retval;
            }
            constexpr const_reverse_char_iterator operator-= (int n) noexcept
            {
                ptr_ += n;
                return *this;
            }

            // TODO: operator<=> () const
            friend constexpr bool operator== (const_reverse_char_iterator lhs, const_reverse_char_iterator rhs) noexcept
            { return lhs.ptr_ == rhs.ptr_; }
            friend constexpr bool operator!= (const_reverse_char_iterator lhs, const_reverse_char_iterator rhs) noexcept
            { return lhs.ptr_ != rhs.ptr_; }
            friend constexpr bool operator< (const_reverse_char_iterator lhs, const_reverse_char_iterator rhs) noexcept
            { return lhs.ptr_ < rhs.ptr_; }
            friend constexpr bool operator<= (const_reverse_char_iterator lhs, const_reverse_char_iterator rhs) noexcept
            { return lhs.ptr_ <= rhs.ptr_; }
            friend constexpr bool operator> (const_reverse_char_iterator lhs, const_reverse_char_iterator rhs) noexcept
            { return lhs.ptr_ > rhs.ptr_; }
            friend constexpr bool operator>= (const_reverse_char_iterator lhs, const_reverse_char_iterator rhs) noexcept
            { return lhs.ptr_ >= rhs.ptr_; }

            friend constexpr const_reverse_char_iterator operator+ (const_reverse_char_iterator lhs, int rhs) noexcept
            { return lhs += rhs; }
            friend constexpr const_reverse_char_iterator operator+ (int lhs, const_reverse_char_iterator rhs) noexcept
            { return rhs += lhs; }
            friend constexpr const_reverse_char_iterator operator- (const_reverse_char_iterator lhs, int rhs) noexcept
            { return lhs -= rhs; }
            friend constexpr const_reverse_char_iterator operator- (int lhs, const_reverse_char_iterator rhs) noexcept
            { return rhs -= lhs; }
            friend constexpr int operator- (const_reverse_char_iterator lhs, const_reverse_char_iterator rhs) noexcept
            { return lhs.ptr_ - rhs.ptr_; }

        private:
            char const * ptr_;
        };

        template <typename Iter>
        struct iterator_range
        {
            Iter first;
            Iter last;

            friend Iter begin (iterator_range r) { return r.first; }
            friend Iter end (iterator_range r) { return r.last; }
        };

    }

    struct text_view
    {
        using iterator = char const *;
        using const_iterator = char const *;
        using reverse_iterator = detail::const_reverse_char_iterator;
        using const_reverse_iterator = detail::const_reverse_char_iterator;

        constexpr text_view () noexcept :
            data_ (nullptr),
            size_ (0)
        {}

        constexpr text_view (char const * c_str) noexcept :
            data_ (c_str),
            size_ (detail::strlen(c_str))
        { assert(detail::strlen(c_str) < INT_MAX); }

        constexpr text_view (char const * c_str, int len) noexcept :
            data_ (c_str),
            size_ (len)
        { assert(0 <= len); }

        constexpr text_view (text const & t) noexcept;

        constexpr const_iterator begin () const noexcept { return data_; }
        constexpr const_iterator end () const noexcept { return data_ + size_; }

        constexpr const_iterator cbegin () const noexcept { return begin(); }
        constexpr const_iterator cend () const noexcept { return end(); }

        constexpr const_reverse_iterator rbegin () const noexcept { return reverse_iterator(end() - 1); }
        constexpr const_reverse_iterator rend () const noexcept { return reverse_iterator(begin() - 1); }

        constexpr const_reverse_iterator crbegin () const noexcept { return rbegin(); }
        constexpr const_reverse_iterator crend () const noexcept { return rend(); }

        constexpr bool empty () const noexcept
        { return size_ == 0; }

        constexpr int size () const noexcept
        { return size_; }

        constexpr char const * data() const noexcept
        { return data_; }

        constexpr char front () const noexcept
        {
            assert(!empty());
            return *data_;
        }

        constexpr char back () const noexcept
        {
            assert(!empty());
            return data_[size_ - 1];
        }

        constexpr char operator[] (int i) const noexcept
        {
            assert(i < size_);
            return data_[i];
        }

        constexpr int max_size () const noexcept
        { return INT_MAX; }

        // TODO: operator<=> () const
        constexpr int compare (text_view rhs) const noexcept
        {
            int const size = detail::min_(size_, rhs.size_);
            if (size == 0)
                return 0;
            int retval = memcmp(data_, rhs.data_, size);
            if (retval == 0) {
                if (size_ < rhs.size_) return -1;
                if (size_ == rhs.size_) return 0;
                return 1;
            }
            return retval;
        }

        constexpr bool operator== (text_view rhs) const noexcept
        { return compare(rhs) == 0; }

        constexpr bool operator!= (text_view rhs) const noexcept
        { return compare(rhs) != 0; }

        constexpr bool operator< (text_view rhs) const noexcept
        { return compare(rhs) < 0; }

        constexpr bool operator<= (text_view rhs) const noexcept
        { return compare(rhs) <= 0; }

        constexpr bool operator> (text_view rhs) const noexcept
        { return compare(rhs) > 0; }

        constexpr bool operator>= (text_view rhs) const noexcept
        { return compare(rhs) >= 0; }

        constexpr void swap (text_view & rhs) noexcept
        {
            {
                char const * tmp = data_;
                data_ = rhs.data_;
                rhs.data_ = tmp;
            }
            {
                int tmp = size_;
                size_ = rhs.size_;
                rhs.size_ = tmp;
            }
        }

        friend iterator begin (text_view v)
        { return v.begin(); }
        friend iterator end (text_view v)
        { return v.end(); }
        friend iterator cbegin (text_view v)
        { return v.cbegin(); }
        friend iterator cend (text_view v)
        { return v.cend(); }

        friend reverse_iterator rbegin (text_view v)
        { return v.rbegin(); }
        friend reverse_iterator rend (text_view v)
        { return v.rend(); }
        friend reverse_iterator crbegin (text_view v)
        { return v.crbegin(); }
        friend reverse_iterator crend (text_view v)
        { return v.crend(); }

    private:
        char const * data_;
        int size_;
    };

    struct rope_view
    {
        // TODO
    };

    struct text
    {
        using iterator = char *;
        using const_iterator = char const *;
        using reverse_iterator = detail::reverse_char_iterator;
        using const_reverse_iterator = detail::const_reverse_char_iterator;

        text () : data_ (nullptr), size_ (0), cap_ (0) {}

        text (text const & t) :
            data_ (new char[t.size()]),
            size_ (t.size()),
            cap_ (t.size())
        { memcpy(data_.get(), t.data(), t.size()); }

        text & operator= (text const & t)
        {
            data_.reset(new char[t.size()]);
            size_ = t.size();
            cap_ = t.size();
            memcpy(data_.get(), t.data(), t.size());
            return *this;
        }

        text (text &&) = default;
        text & operator= (text &&) = default;

        text (char const * c_str) :
            data_ (nullptr),
            size_ (0),
            cap_ (0)
        {
            auto len = strlen(c_str) + 1;
            if (INT_MAX < len)
                throw std::bad_alloc();
            data_.reset(new char[len]);
            memcpy(data_.get(), c_str, len);
            size_ = (int)len;
        }

        text (text_view view) :
            data_ (new char[view.size()]),
            size_ (view.size()),
            cap_ (view.size())
        { memcpy(data_.get(), view.data(), view.size()); }

        const_iterator begin () const { return data_.get(); }
        const_iterator end () const { return data_.get() + size_; }

        iterator begin () { return data_.get(); }
        iterator end () { return data_.get() + size_; }

        bool empty () const
        { return size_ == 0; }

        int size () const
        { return size_; }

        char const * data() const
        { return data_.get(); }

        friend iterator begin (text & t)
        { return t.begin(); }
        friend iterator end (text & t)
        { return t.end(); }

        friend const_iterator begin (text const & t)
        { return t.begin(); }
        friend const_iterator end (text const & t)
        { return t.end(); }

    private:
        std::unique_ptr<char[]> data_;
        int size_;
        int cap_;
    };

    inline constexpr text_view::text_view (text const & t) noexcept :
        data_ (t.data()),
        size_ (t.size())
    {}

    // TODO: Use this in text_view::compare().
    template <typename LCharRange, typename RCharRange>
    constexpr int compare (LCharRange const & l, RCharRange const & r) noexcept
    {
        auto const l_it = begin(l);
        auto const r_it = begin(r);
        auto const l_size = end(l) - l_it;
        auto const r_size = end(r) - r_it;
        assert(l_size <= INT_MAX);
        assert(r_size <= INT_MAX);
        int const size = (int)detail::min_(l_size, r_size);
        if (size == 0)
            return 0;
        int retval = memcmp(&*l_it, &*r_it, size);
        if (retval == 0) {
            if (l_size < r_size) return -1;
            if (l_size == r_size) return 0;
            return 1;
        }
        return retval;
    }

    // TODO: Constrain.
    template <typename CharRange, typename PatternCharRange>
    constexpr text_view find (CharRange const & r, PatternCharRange const & p) noexcept
    {
        char const * r_first = &*begin(r);
        char const * p_first = &*begin(p);
        char const * const r_last = &*end(r);
        char const * const p_last = &*end(p);

        if (r_first == r_last)
            return text_view(&*r_last, 0);
        if (p_first == p_last)
            return text_view(&*r_first, 0);

        char const p_head = *p_first;
        int r_len = r_last - r_first;
        int const p_len = p_last - p_first;
        while (true) {
            r_len = r_last - r_first;
            if (r_len < p_len)
                return text_view(&*r_last, 0);

            r_first = strchr(r_first, p_head);
            if (r_first == nullptr)
                return text_view(&*r_last, 0);

            text_view candidate(r_first, p_len);
            if (compare(candidate, p) == 0)
                return candidate;

            ++r_first;
        }
    }

    // TODO: Constrain.
    template <typename CharRange, typename PatternCharRange>
    constexpr text_view rfind (CharRange const & r, PatternCharRange const & p) noexcept
    {
        char const * const r_first = &*begin(r);
        char const * p_first = &*begin(p);
        char const * r_last = &*end(r);
        char const * const p_last = &*end(p);

        if (r_first == r_last)
            return text_view(&*r_last, 0);
        if (p_first == p_last)
            return text_view(&*r_first, 0);

        char const p_head = *p_first;
        int r_len = r_last - r_first;
        int const p_len = p_last - p_first;
        while (true) {
            r_len = r_last - r_first;
            if (r_len < p_len)
                return text_view(&*r_last, 0);

            auto candidate_first = detail::strrchr(r_first, r_last, p_head);
            if (candidate_first == nullptr)
                return text_view(&*r_last, 0);

            text_view candidate(candidate_first, p_len);
            if (compare(candidate, p) == 0)
                return candidate;

            r_last = candidate_first;
        }
    }

    template <typename CharRange>
    constexpr text_view substr (CharRange const & r, int start, int size) noexcept
    {
        assert(start + size <= r.size());
        return text_view(&*begin(r) + start, size);
    }

} }

/* Rationale

   1: use of int for sizes

   2: begin and end free functions, but no others (you can make the others
   from these)

*/

#endif
