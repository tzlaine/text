#ifndef BOOST_TEXT_TEXT_HPP
#define BOOST_TEXT_TEXT_HPP

#include <boost/text/utf8.hpp>

#include <boost/text/detail/algorithm.hpp>
#include <boost/text/detail/iterator.hpp>
#include <boost/text/detail/utility.hpp>

#include <list>
#include <memory>

#include <cassert>


namespace boost { namespace text {

    struct text_view;
    struct repeated_text_view;

    // TODO: text should use the more efficient versions of the
    // constexpr-friendly-but-slower operations that text_view does.
    struct text
    {
        using iterator = char *;
        using const_iterator = char const *;
        using reverse_iterator = detail::reverse_char_iterator;
        using const_reverse_iterator = detail::const_reverse_char_iterator;

        text () noexcept : data_ (), size_ (0), cap_ (0) {}

        text (text const & t) : data_ (), size_ (0), cap_ (0)
        { insert(0, t); }

        text (text && rhs) noexcept : data_ (), size_ (0), cap_ (0)
        { swap(rhs); }

        template <typename CharRange>
        explicit text (
            CharRange const & r,
            detail::rng_alg_ret_t<int *, CharRange> = 0
        ) : data_ (), size_ (0), cap_ (0)
        { insert(0, r); }

        inline explicit text (text_view tv);
        inline explicit text (repeated_text_view tv);

        template <typename Iter>
        text (
            Iter first, Iter last,
            detail::char_iter_ret_t<void *, Iter> = 0
        ) : data_ (), size_ (0), cap_ (0)
        { insert(0, first, last); }

        text & operator= (text const & t)
        {
            if (t.size() <= size()) {
                clear();
                insert(0, t);
            } else {
                text tmp(t);
                swap(tmp);
            }
            return *this;
        }

        text & operator= (text && rhs) noexcept
        {
            swap(rhs);
            return *this;
        }

        template <typename CharRange>
        auto operator= (CharRange const & r)
            -> detail::rng_alg_ret_t<text &, CharRange>;

        inline text & operator= (text_view tv);
        inline text & operator= (repeated_text_view tv);

        iterator begin () noexcept
        {
            if (!data_)
                return reinterpret_cast<char *>(&cap_);
            return data_.get();
        }
        iterator end () noexcept
        {
            if (!data_)
                return reinterpret_cast<char *>(&cap_);
            return data_.get() + size_;
        }

        const_iterator begin () const noexcept
        {
            if (!data_)
                return reinterpret_cast<char const *>(&cap_);
            return data_.get();
        }
        const_iterator end () const noexcept
        {
            if (!data_)
                return reinterpret_cast<char const *>(&cap_);
            return data_.get() + size_;
        }

        const_iterator cbegin () const noexcept { return begin(); }
        const_iterator cend () const noexcept { return end(); }

        reverse_iterator rbegin () noexcept { return reverse_iterator(end() - 1); }
        reverse_iterator rend () noexcept { return reverse_iterator(begin() - 1); }

        const_reverse_iterator rbegin () const noexcept { return const_reverse_iterator(end()); }
        const_reverse_iterator rend () const noexcept { return const_reverse_iterator(begin()); }

        const_reverse_iterator crbegin () const noexcept { return const_reverse_iterator(end()); }
        const_reverse_iterator crend () const noexcept { return const_reverse_iterator(begin()); }

        bool empty () const noexcept
        { return size_ == 0; }

        int size () const noexcept
        { return size_; }

        int capacity () const noexcept
        { return cap_ - 1; }

        char operator[] (int i) const noexcept
        {
#ifndef BOOST_TEXT_TESTING
            assert(0 <= 0 && i < size_);
#endif
            return data_[i];
        }

        text_view operator() (int lo, int hi) const;
        text_view operator() (int cut) const;

        int max_size () const noexcept
        { return INT_MAX; }

        int compare (text_view rhs) const noexcept;

        bool operator== (text_view rhs) const noexcept;
        bool operator!= (text_view rhs) const noexcept;
        bool operator< (text_view rhs) const noexcept;
        bool operator<= (text_view rhs) const noexcept;
        bool operator> (text_view rhs) const noexcept;
        bool operator>= (text_view rhs) const noexcept;

        void clear () noexcept
        {
            size_ = 0;
            if (data_)
                data_[0] = '\0';
        }

        char & operator[] (int i) noexcept
        {
#ifndef BOOST_TEXT_TESTING
            assert(0 <= 0 && i < size_);
#endif
            return data_[i];
        }

        template <typename CharRange>
        auto insert (int at, CharRange const & r)
            -> detail::rng_alg_ret_t<text &, CharRange>;

        inline text & insert (int at, text_view tv);
        inline text & insert (int at, repeated_text_view rtv);

        // TODO: Document that the inserted/replaced sequence need not be
        // UTF-8 encoded, since direct use of iterators is the unsafe
        // interface.  (To once again make it safe, use one of the converting
        // iterators.)
        template <typename Iter>
        auto insert (int at, Iter first, Iter last)
            -> detail::char_iter_ret_t<text &, Iter>
        {
            assert(0 <= at && at <= size_);

            if (first == last)
                return *this;

            if (!utf8::starts_encoded(cbegin() + at, cend()))
                throw std::invalid_argument("Inserting at that character breaks UTF-8 encoding.");

            return insert_iter_impl(at, first, last);
        }

        template <typename Iter>
        auto insert (iterator at, Iter first, Iter last)
            -> detail::char_iter_ret_t<text &, Iter>
        {
            assert(begin() <= at && at <= end());

            if (first == last)
                return *this;

            return insert_iter_impl(at - begin(), first, last);
        }

        inline text & erase (text_view tv) noexcept;

        text & erase (iterator first, iterator last) noexcept
        {
            assert(first <= last);
            assert(begin() <= first && last <= end());

            std::copy(last, end(), first);
            size_ -= last - first;
            data_[size_] = '\0';

            return *this;
        }

        template <typename CharRange>
        auto replace (text_view old_substr, CharRange const & r)
            -> detail::rng_alg_ret_t<text &, CharRange>;

        inline text & replace (text_view old_substr, text_view new_substr);
        inline text & replace (text_view old_substr, repeated_text_view new_substr);

        template <typename Iter>
        auto replace (text_view old_substr, Iter first, Iter last)
            -> detail::char_iter_ret_t<text &, Iter>;

        // TODO: Perf test replace(Iter) against insert(Iter), and replace the
        // insert(Iter) implementation if that is warranted.
        template <typename Iter>
        auto replace (iterator old_first, iterator old_last, Iter new_first, Iter new_last)
            -> detail::char_iter_ret_t<text &, Iter>
        {
            assert(begin() <= old_first && old_last <= end());

            char stack_buf[1024];
            std::list<text> heap_bufs;
            int const chars_pushed =
                read_iters(stack_buf, sizeof(stack_buf), heap_bufs, new_first, new_last);
            int const stack_buf_bytes = (std::min)(chars_pushed, (int)sizeof(stack_buf));

            int const delta = chars_pushed - (old_last - old_first);
            int const available = cap_ - 1 - size_;
            if (available < delta) {
                std::unique_ptr<char []> new_data = get_new_data(delta - available);
                char * buf = new_data.get();
                buf = std::copy(begin(), old_first, buf);
                buf = copy_bufs(stack_buf, stack_buf_bytes, heap_bufs, buf);
                std::copy(old_last, end(), buf);
                new_data.swap(data_);
            } else {
                if (0 < delta)
                    std::copy_backward(old_last, end(), end() + delta);
                else if (delta < 0)
                    std::copy(old_last, end(), old_last + delta);
                copy_bufs(stack_buf, stack_buf_bytes, heap_bufs, old_first);
            }

            size_ += delta;
            data_[size_] = '\0';

            return *this;
        }

        void resize (int new_size, char c)
        {
            assert(0 <= new_size);

            if (c & 0x80)
                throw std::invalid_argument("Given character is not a valid UTF-8 1-character code point");

            int const prev_size = size_;
            int const delta = new_size - prev_size;
            if (!delta)
                return;

            int const available = cap_ - 1 - size_;
            if (available < delta) {
                std::unique_ptr<char []> new_data = get_new_data(delta - available);
                std::copy(begin(), begin() + prev_size, new_data.get());
                new_data.swap(data_);
            } else {
                if (!utf8::ends_encoded(cbegin(), cbegin() + new_size))
                    throw std::invalid_argument("Resizing to the given size breaks UTF-8 encoding.");
            }

            size_ = new_size;

            if (0 < delta)
                std::fill(begin() + prev_size, end(), c);

            data_[size_] = '\0';
        }

        void reserve (int new_size)
        {
            assert(0 <= new_size);
            int const new_cap = new_size + 1;
            if (new_cap <= cap_)
                return;
            std::unique_ptr<char []> new_data(new char[new_cap]);
            *std::copy(cbegin(), cend(), new_data.get()) = '\0';
            data_.swap(new_data);
            cap_ = new_cap;
        }

        void shrink_to_fit ()
        {
            if (cap_ == 0 || cap_ == size_ + 1)
                return;
            std::unique_ptr<char []> new_data(new char[size_ + 1]);
            *std::copy(cbegin(), cend(), new_data.get()) = '\0';
            data_.swap(new_data);
            cap_ = size_ + 1;
        }

        void swap (text & rhs) noexcept
        {
            data_.swap(rhs.data_);
            std::swap(size_, rhs.size_);
            std::swap(cap_, rhs.cap_);
        }

        inline text & operator+= (text_view tv);
        inline text & operator+= (repeated_text_view rtv);

        template <typename CharRange>
        auto operator+= (CharRange const & r)
            -> detail::rng_alg_ret_t<text &, CharRange>;

        friend iterator begin (text & t) noexcept
        { return t.begin(); }
        friend iterator end (text & t) noexcept
        { return t.end(); }
        friend const_iterator begin (text const & t) noexcept
        { return t.begin(); }
        friend const_iterator end (text const & t) noexcept
        { return t.end(); }
        friend const_iterator cbegin (text const & t) noexcept
        { return t.cbegin(); }
        friend const_iterator cend (text const & t) noexcept
        { return t.cend(); }

        friend reverse_iterator rbegin (text & t) noexcept
        { return t.rbegin(); }
        friend reverse_iterator rend (text & t) noexcept
        { return t.rend(); }
        friend const_reverse_iterator rbegin (text const & t) noexcept
        { return t.rbegin(); }
        friend const_reverse_iterator rend (text const & t) noexcept
        { return t.rend(); }
        friend const_reverse_iterator crbegin (text const & t) noexcept
        { return t.crbegin(); }
        friend const_reverse_iterator crend (text const & t) noexcept
        { return t.crend(); }

        friend std::ostream & operator<< (std::ostream & os, text const & t)
        {
            if (os.good()) {
                detail::pad_width_before(os, t.size());
                if (os.good())
                    os.write(t.begin(), t.size());
                if (os.good())
                    detail::pad_width_after(os, t.size());
                os.width(0);
            }
            return os;
        }

    private:
        bool self_reference (text_view tv) const;

        int grow_cap (int min_new_cap) const
        {
            assert(0 < min_new_cap);
            int retval = (std::max)(8, cap_);
            while (retval < min_new_cap) {
                retval = retval / 2 * 3;
            }
            // Leave very short strings very short (8 bytes -- even though
            // this is not portably achievable), but have the rest end on a
            // 16-byte bundary.
            if (8 < retval) {
                int const rem = (retval + 16) % 16;
                retval += 16 - rem;
            }
            return retval;
        }

        std::unique_ptr<char []> get_new_data (int resize_amount)
        {
            int const new_cap =
                0 < resize_amount ? grow_cap(cap_ + resize_amount) : cap_;
            std::unique_ptr<char []> retval(new char [new_cap]);
            cap_ = new_cap;
            return retval;
        }

        void push_char (char c, std::unique_ptr<char []> & initial_data)
        {
            int const available = cap_ - 1 - size_;
            if (available < 1) {
                std::unique_ptr<char []> new_data = get_new_data(1 - available);
                std::copy(cbegin(), cend(), new_data.get());
                if (!initial_data) {
                    initial_data = std::move(data_);
                    data_ = std::move(new_data);
                } else {
                    new_data.swap(data_);
                }
            }
            data_[size_] = c;
            ++size_;
        }

        template <typename Iter>
        auto insert_iter_impl (int at, Iter first, Iter last)
            -> detail::char_iter_ret_t<text &, Iter>
        {
            std::unique_ptr<char []> initial_data;
            int const initial_size = size_;
            int const initial_cap = cap_;
            try {
                while (first != last) {
                    push_char(*first, initial_data);
                    ++first;
                }
            } catch (std::bad_alloc const &) {
                data_.swap(initial_data);
                size_ = initial_size;
                cap_ = initial_cap;
                throw;
            }

            std::rotate(begin() + at, begin() + initial_size, end());
            data_[size_] = '\0';

            return *this;
        }

        template <typename Iter>
        struct buf_ptr_iterator
        {
            char * buf_;
            Iter it_;
        };

        template <typename Iter>
        buf_ptr_iterator<Iter> fill_buf (char * buf, int size, Iter first, Iter last)
        {
            char * const buf_end = buf + size;
            while (first != last && buf != buf_end) {
                *buf = *first;
                ++buf;
                ++first;
            }
            return {buf, first};
        }

        char * copy_bufs (char * buf, int size, std::list<text> const & bufs, char * it)
        {
            it = std::copy_n(buf, size, it);
            for (text const & t : bufs) {
                it = std::copy_n(t.data_.get(), t.size_, it);
            }
            return it;
        }

        template <typename Iter>
        int read_iters (char * buf, int size, std::list<text> & bufs, Iter first, Iter last)
        {
            buf_ptr_iterator<Iter> buf_first = fill_buf(buf, size, first, last);

            int chars_pushed = buf_first.buf_ - buf;
            int buf_size = size;

            while (buf_first.it_ != last) {
                buf_size *= 2;
                bufs.push_back(text());
                text & temp = bufs.back();
                temp.data_.reset(new char[buf_size]);
                temp.cap_ = buf_size;
                buf_first = fill_buf(temp.data_.get(), buf_size, buf_first.it_, last);
                temp.size_ = buf_first.buf_ - temp.data_.get();
                chars_pushed += temp.size_;
            }

            return chars_pushed;
        }

        std::unique_ptr<char []> data_;
        int size_;
        int cap_;
    };

    inline text const & checked_encoding (text const & t)
    {
        if (!utf8::encoded(t.begin(), t.end()))
            throw std::invalid_argument("Invalid UTF-8 encoding");
        return t;
    }

    inline text && checked_encoding (text && t)
    {
        if (!utf8::encoded(t.begin(), t.end()))
            throw std::invalid_argument("Invalid UTF-8 encoding");
        return std::move(t);
    }

} }

#include <boost/text/text_view.hpp>

namespace boost { namespace text {

    namespace literals {

        inline text operator"" _t (char const * str, std::size_t len)
        {
            assert(len < INT_MAX);
            return text(text_view(str, len));
        }

        inline text operator"" _t (char16_t const * str, std::size_t len)
        {
            assert(len < INT_MAX / 2);
            return text(
                utf8::from_utf16_iterator<char16_t const *>(str),
                utf8::from_utf16_iterator<char16_t const *>(str + len)
            );
        }

        inline text operator"" _t (char32_t const * str, std::size_t len)
        {
            assert(len < INT_MAX / 4);
            return text(
                utf8::from_utf32_iterator<char32_t const *>(str),
                utf8::from_utf32_iterator<char32_t const *>(str + len)
            );
        }

    }

    inline text::text (text_view tv) : data_ (), size_ (0), cap_ (0)
    { insert(0, tv); }

    inline text::text (repeated_text_view rtv) : data_ (), size_ (0), cap_ (0)
    { insert(0, rtv); }

    template <typename CharRange>
    auto text::operator= (CharRange const & r)
        -> detail::rng_alg_ret_t<text &, CharRange>
    { return *this = text_view(&*r.begin(), r.end() - r.begin()); }

    inline text & text::operator= (text_view tv)
    {
        assert(0 <= tv.size());
        bool const self_ref = self_reference(tv);
        if (self_ref) {
            erase(text_view(tv.end(), end() - tv.end()));
            erase(text_view(begin(), tv.begin() - begin()));
        } else if (tv.size() <= size()) {
            clear();
            insert(0, tv);
        } else {
            text tmp(tv);
            swap(tmp);
        }
        return *this;
    }

    inline text & text::operator= (repeated_text_view rtv)
    {
        assert(0 <= rtv.size());
        bool const self_ref = self_reference(rtv.view());
        if (!self_ref && rtv.size() <= size()) {
            clear();
            insert(0, rtv);
        } else {
            text tmp(rtv);
            swap(tmp);
        }
        return *this;
    }

    inline text_view text::operator() (int lo, int hi) const
    { return text_view(*this)(lo, hi); }

    inline text_view text::operator() (int cut) const
    { return text_view(*this)(cut); }

    inline int text::compare (text_view rhs) const noexcept
    { return detail::compare_impl(begin(), end(), rhs.begin(), rhs.end()); }

    inline bool text::operator== (text_view rhs) const noexcept
    { return compare(rhs) == 0; }

    inline bool text::operator!= (text_view rhs) const noexcept
    { return compare(rhs) != 0; }

    inline bool text::operator< (text_view rhs) const noexcept
    { return compare(rhs) < 0; }

    inline bool text::operator<= (text_view rhs) const noexcept
    { return compare(rhs) <= 0; }

    inline bool text::operator> (text_view rhs) const noexcept
    { return compare(rhs) > 0; }

    inline bool text::operator>= (text_view rhs) const noexcept
    { return compare(rhs) >= 0; }

    template <typename CharRange>
    auto text::insert (int at, CharRange const & r)
        -> detail::rng_alg_ret_t<text &, CharRange>
    { return insert(at, text_view(&*r.begin(), r.end() - r.begin())); }

    inline text & text::insert (int at, text_view tv)
    {
        assert(0 <= at && at <= size_);
        assert(0 <= tv.size());

        if (!utf8::starts_encoded(cbegin() + at, cend()))
            throw std::invalid_argument("Inserting at that character breaks UTF-8 encoding.");

        bool const tv_null_terminated = !tv.empty() && tv.end()[-1] == '\0';
        if (tv_null_terminated)
            tv = tv(0, -1);

        int const delta = tv.size();
        if (!delta)
            return *this;

        bool const late_self_ref =
            self_reference(tv) && at < tv.end() - begin();
        int const available = cap_ - 1 - size_;
        if (late_self_ref || available < delta) {
            std::unique_ptr<char []> new_data = get_new_data(delta - available);
            char * buf = new_data.get();
            buf = std::copy(cbegin(), cbegin() + at, buf);
            buf = std::copy(tv.begin(), tv.end(), buf);
            buf = std::copy(cbegin() + at, cend(), buf);
            new_data.swap(data_);
        } else {
            std::copy_backward(cbegin() + at, cend(), end() + delta);
            char * buf = begin() + at;
            std::copy(tv.begin(), tv.end(), buf);
        }

        size_ += delta;
        data_[size_] = '\0';

        return *this;
    }

    inline text & text::insert (int at, repeated_text_view rtv)
    {
        assert(0 <= at && at <= size_);
        assert(0 <= rtv.size());

        if (!utf8::starts_encoded(cbegin() + at, cend()))
            throw std::invalid_argument("Inserting at that character breaks UTF-8 encoding.");

        bool const rtv_null_terminated = !rtv.view().empty() && rtv.view().end()[-1] == '\0';
        if (rtv_null_terminated)
            rtv = repeat(rtv.view()(0, -1), rtv.count());

        int const delta = rtv.size();
        if (!delta)
            return *this;

        bool const late_self_ref =
            self_reference(rtv.view()) && at < rtv.view().end() - begin();
        int const available = cap_ - 1 - size_;
        if (late_self_ref || available < delta) {
            std::unique_ptr<char []> new_data = get_new_data(delta - available);
            char * buf = new_data.get();
            buf = std::copy(cbegin(), cbegin() + at, buf);
            for (int i = 0; i < rtv.count(); ++i) {
                buf = std::copy(rtv.view().begin(), rtv.view().end(), buf);
            }
            std::copy(cbegin() + at, cend(), buf);
            new_data.swap(data_);
        } else {
            std::copy_backward(cbegin() + at, cend(), end() + delta);
            char * buf = begin() + at;
            for (int i = 0; i < rtv.count(); ++i) {
                buf = std::copy(rtv.view().begin(), rtv.view().end(), buf);
            }
        }

        size_ += delta;
        data_[size_] = '\0';

        return *this;
    }

    inline text & text::erase (text_view tv) noexcept
    {
        assert(0 <= tv.size());

        bool const tv_null_terminated = !tv.empty() && tv.end()[-1] == '\0';
        if (tv_null_terminated)
            tv = tv(0, -1);

        char * first = const_cast<char *>(tv.begin());
        return erase(first, first + tv.size());
    }

    template <typename CharRange>
    auto text::replace (text_view old_substr, CharRange const & r)
        -> detail::rng_alg_ret_t<text &, CharRange>
    { return replace(old_substr, text_view(&*r.begin(), r.end() - r.begin())); }

    inline text & text::replace (text_view old_substr, text_view new_substr)
    {
        assert(0 <= old_substr.size());
        assert(0 <= new_substr.size());

        bool const old_substr_null_terminated =
            !old_substr.empty() && old_substr.end()[-1] == '\0';
        if (old_substr_null_terminated)
            old_substr = old_substr(0, -1);

        assert(self_reference(old_substr));

        if (old_substr.empty()) {
            if (old_substr.begin() == begin()) {
                text_view check_after(old_substr.begin(), end() - old_substr.begin());
                (void)check_after;
            } else {
                text_view check_before(data_.get(), old_substr.begin() - begin());
                (void)check_before;
            }
        }

        bool const new_substr_null_terminated =
            !new_substr.empty() && new_substr.end()[-1] == '\0';
        if (new_substr_null_terminated)
            new_substr = new_substr(0, -1);

        assert(begin() <= old_substr.begin() && old_substr.end() <= end());

        bool const late_self_ref =
            self_reference(new_substr) && old_substr.begin() < new_substr.end();
        int const delta = new_substr.size() - old_substr.size();
        int const available = cap_ - 1 - size_;
        if (late_self_ref || available < delta) {
            std::unique_ptr<char []> new_data = get_new_data(delta - available);
            char * buf = new_data.get();
            buf = std::copy(cbegin(), old_substr.begin(), buf);
            buf = std::copy(new_substr.begin(), new_substr.end(), buf);
            std::copy(old_substr.end(), cend(), buf);
            new_data.swap(data_);
        } else {
            if (0 < delta) {
                std::copy_backward(
                    old_substr.end(), cend(),
                    end() + delta
                );
            } else if (delta < 0) {
                std::copy(
                    old_substr.end(), cend(),
                    const_cast<char *>(old_substr.end()) + delta
                );
            }
            char * buf = const_cast<char *>(old_substr.begin());
            std::copy(new_substr.begin(), new_substr.end(), buf);
        }

        size_ += delta;
        data_[size_] = '\0';

        return *this;
    }

    inline text & text::replace (text_view old_substr, repeated_text_view new_substr)
    {
        assert(0 <= old_substr.size());
        assert(0 <= new_substr.size());

        bool const old_substr_null_terminated =
            !old_substr.empty() && old_substr.end()[-1] == '\0';
        if (old_substr_null_terminated)
            old_substr = old_substr(0, -1);

        assert(self_reference(old_substr));

        if (old_substr.empty()) {
            if (old_substr.begin() == begin()) {
                text_view check_after(old_substr.begin(), end() - old_substr.begin());
                (void)check_after;
            } else {
                text_view check_before(data_.get(), old_substr.begin() - begin());
                (void)check_before;
            }
        }

        bool const new_substr_null_terminated =
            !new_substr.view().empty() && new_substr.view().end()[-1] == '\0';
        if (new_substr_null_terminated)
            new_substr = repeat(new_substr.view()(0, -1), new_substr.count());

        assert(begin() <= old_substr.begin() && old_substr.end() <= end());

        bool const late_self_ref =
            self_reference(new_substr.view()) && old_substr.begin() < new_substr.view().end();
        int const delta = new_substr.size() - old_substr.size();
        int const available = cap_ - 1 - size_;
        if (late_self_ref || available < delta) {
            std::unique_ptr<char []> new_data = get_new_data(delta - available);
            char * buf = new_data.get();
            buf = std::copy(cbegin(), old_substr.begin(), buf);
            for (int i = 0; i < new_substr.count(); ++i) {
                buf = std::copy(new_substr.view().begin(), new_substr.view().end(), buf);
            }
            std::copy(old_substr.end(), cend(), buf);
            new_data.swap(data_);
        } else {
            if (0 < delta) {
                std::copy_backward(
                    old_substr.end(), cend(),
                    end() + delta
                );
            } else if (delta < 0) {
                std::copy(
                    old_substr.end(), cend(),
                    const_cast<char *>(old_substr.end()) + delta
                );
            }
            char * buf = const_cast<char *>(old_substr.begin());
            for (int i = 0; i < new_substr.count(); ++i) {
                buf = std::copy(new_substr.view().begin(), new_substr.view().end(), buf);
            }
        }

        size_ += delta;
        data_[size_] = '\0';

        return *this;
    }

    // TODO: Add a noncontiguous random_access Iter overload, here and elsewhere.
    template <typename Iter>
    auto text::replace (text_view old_substr, Iter first, Iter last)
        -> detail::char_iter_ret_t<text &, Iter>
    {
        assert(0 <= old_substr.size());

        bool const old_substr_null_terminated =
            !old_substr.empty() && old_substr.end()[-1] == '\0';
        if (old_substr_null_terminated)
            old_substr = old_substr(0, -1);

        char * old_first = const_cast<char *>(old_substr.begin());
        return replace(old_first, old_first + old_substr.size(), first, last);
    }

    inline text & text::operator+= (text_view tv)
    { return insert(size(), tv); }

    inline text & text::operator+= (repeated_text_view rtv)
    {
        assert(0 <= rtv.size());
        reserve(size() + rtv.size());
        for (std::ptrdiff_t i = 0; i < rtv.count(); ++i) {
            insert(size(), rtv.view());
        }
        return *this;
    }

    template <typename CharRange>
    auto text::operator+= (CharRange const & r)
        -> detail::rng_alg_ret_t<text &, CharRange>
    { return insert(size(), text_view(&*r.begin(), r.end() - r.begin())); }

    inline bool text::self_reference (text_view tv) const
    {
        using less_t = std::less<char const *>;
        less_t less;
        return !less(tv.begin(), begin()) && !less(end(), tv.end());
    }


    inline bool operator== (char const * lhs, text const & rhs) noexcept
    { return detail::compare_impl(lhs, lhs + strlen(lhs), rhs.begin(), rhs.end()) == 0; }

    inline bool operator!= (char const * lhs, text const & rhs) noexcept
    { return detail::compare_impl(lhs, lhs + strlen(lhs), rhs.begin(), rhs.end()) != 0; }

    inline bool operator< (char const * lhs, text const & rhs) noexcept
    { return detail::compare_impl(lhs, lhs + strlen(lhs), rhs.begin(), rhs.end()) < 0; }

    inline bool operator<= (char const * lhs, text const & rhs) noexcept
    { return detail::compare_impl(lhs, lhs + strlen(lhs), rhs.begin(), rhs.end()) <= 0; }

    inline bool operator> (char const * lhs, text const & rhs) noexcept
    { return detail::compare_impl(lhs, lhs + strlen(lhs), rhs.begin(), rhs.end()) > 0; }

    inline bool operator>= (char const * lhs, text const & rhs) noexcept
    { return detail::compare_impl(lhs, lhs + strlen(lhs), rhs.begin(), rhs.end()) >= 0; }



    inline text operator+ (text t, text_view tv)
    { return t += tv; }

    inline text operator+ (text t, repeated_text_view rtv)
    { return t += rtv; }

    template <typename CharRange>
    auto operator+ (text t, CharRange const & r)
        -> detail::rng_alg_ret_t<text, CharRange>
    { return t += r; }

    inline text operator+ (text_view tv, text const & t)
    { return (text() += tv) += t; }

    inline text operator+ (repeated_text_view rtv, text const & t)
    { return (text() += rtv) += t; }

    template <typename CharRange>
    auto operator+ (CharRange const & r, text const & t)
        -> detail::rng_alg_ret_t<text, CharRange>
    { return (text() += r) += t; }

} }

#endif
