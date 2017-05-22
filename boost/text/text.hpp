#ifndef BOOST_TEXT_TEXT_HPP
#define BOOST_TEXT_TEXT_HPP

#include <boost/text/text_view.hpp>

#include <memory>


namespace boost { namespace text {

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
    constexpr int find (CharRange const & r, PatternCharRange const & p) noexcept
    {
        char const * const r_first = &*begin(r);
        char const * const r_last = &*end(r);

        if (r_first == r_last)
            return -1;

        char const * const p_last = &*end(p);
        char const * const p_first = &*begin(p);

        if (p_first == p_last)
            return -1;

        char const p_head = *p_first;
        int const p_len = p_last - p_first;
        const char * it = r_first;
        while (true) {
            int const r_len = r_last - it;
            if (r_len < p_len)
                return -1;

            it = strchr(it, p_head);
            if (it == nullptr)
                return -1;

            text_view candidate(it, p_len);
            if (compare(candidate, p) == 0)
                return it - r_first;

            ++it;
        }
    }

    // TODO: Constrain.
    template <typename CharRange, typename PatternCharRange>
    constexpr text_view find_view (CharRange const & r, PatternCharRange const & p) noexcept
    {
        char const * const r_first = &*begin(r);
        char const * const r_last = &*end(r);

        if (r_first == r_last)
            return text_view(r_first, 0);

        char const * const p_last = &*end(p);
        char const * const p_first = &*begin(p);

        if (p_first == p_last)
            return text_view(r_first, 0);

        int const n = find(r, p);
        if (n < 0)
            return text_view(r_last, 0);
        return text_view(r_first + n, p_last - p_first);
    }

    // TODO: Constrain.
    template <typename CharRange, typename PatternCharRange>
    constexpr int find_first_of (CharRange const & r, PatternCharRange const & p) noexcept
    {
        char const * const r_first = &*begin(r);
        char const * const r_last = &*end(r);

        if (r_first == r_last)
            return -1;

        if (begin(p) == end(p))
            return -1;

        char const * it = r_first;
        while (it != r_last) {
            for (char const c : p) {
                if (*it == c)
                    return it - r_first;
            }
            ++it;
        }
        return -1;
    }

    // TODO: Constrain.
    template <typename CharRange, typename PatternCharRange>
    constexpr int find_last_of (CharRange const & r, PatternCharRange const & p) noexcept
    {
        char const * const r_first = &*begin(r);
        char const * const r_last = &*end(r);

        if (r_first == r_last)
            return -1;

        if (begin(p) == end(p))
            return -1;

        char const * it = r_last;
        while (r_first != it) {
            --it;
            for (char const c : p) {
                if (*it == c)
                    return it - r_first;
            }
        }
        return -1;
    }

    // TODO: Constrain.
    template <typename CharRange, typename PatternCharRange>
    constexpr int find_first_not_of (CharRange const & r, PatternCharRange const & p) noexcept
    {
        char const * const r_first = &*begin(r);
        char const * const r_last = &*end(r);

        if (r_first == r_last)
            return -1;

        if (begin(p) == end(p))
            return -1;

        char const * it = r_first;
        while (it != r_last) {
            for (char const c : p) {
                if (*it != c)
                    return it - r_first;
            }
            ++it;
        }
        return -1;
    }

    // TODO: Constrain.
    template <typename CharRange, typename PatternCharRange>
    constexpr int find_last_not_of (CharRange const & r, PatternCharRange const & p) noexcept
    {
        char const * const r_first = &*begin(r);
        char const * const r_last = &*end(r);

        if (r_first == r_last)
            return -1;

        if (begin(p) == end(p))
            return -1;

        char const * it = r_last;
        while (r_first != it) {
            --it;
            for (char const c : p) {
                if (*it != c)
                    return it - r_first;
            }
        }
        return -1;
    }

    // TODO: Constrain.
    template <typename CharRange, typename PatternCharRange>
    constexpr int rfind (CharRange const & r, PatternCharRange const & p) noexcept
    {
        char const * const r_first = &*begin(r);
        char const * const r_last = &*end(r);

        if (r_first == r_last)
            return -1;

        char const * const p_last = &*end(p);
        char const * const p_first = &*begin(p);

        if (p_first == p_last)
            return -1;

        char const p_head = *p_first;
        int const p_len = p_last - p_first;
        const char * it = r_last;
        while (true) {
            int const r_len = it - r_first;
            if (r_len < p_len)
                return -1;

            auto candidate_first = detail::strrchr(r_first, it, p_head);
            if (candidate_first == nullptr)
                return -1;

            text_view candidate(candidate_first, p_len);
            if (compare(candidate, p) == 0)
                return candidate_first - r_first;

            it = candidate_first;
        }
    }

    // TODO: Constrain.
    template <typename CharRange, typename PatternCharRange>
    constexpr text_view rfind_view (CharRange const & r, PatternCharRange const & p) noexcept
    {
        char const * const r_first = &*begin(r);
        char const * const r_last = &*end(r);

        if (r_first == r_last)
            return text_view(r_first, 0);

        char const * const p_last = &*end(p);
        char const * const p_first = &*begin(p);

        if (p_first == p_last)
            return text_view(r_first, 0);

        int const n = rfind(r, p);
        if (n < 0)
            return text_view(r_last, 0);
        return text_view(r_first + n, p_last - p_first);
    }

    template <typename CharRange>
    constexpr text_view substr (CharRange const & r, int start, int size) noexcept
    {
        assert(start + size <= r.size());
        return text_view(&*begin(r) + start, size);
    }

} }

#endif
