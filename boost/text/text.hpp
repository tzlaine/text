#ifndef BOOST_TEXT_TEXT_HPP
#define BOOST_TEXT_TEXT_HPP

#include <boost/text/config.hpp>

#include <boost/text/detail/algorithm.hpp>
#include <boost/text/detail/iterator.hpp>
#include <boost/text/detail/utility.hpp>

#include <algorithm>
#include <array>
#include <list>
#include <memory>

#include <cassert>


namespace boost { namespace text {

    struct text_view;
    struct repeated_text_view;
    struct unencoded_rope;
    struct unencoded_rope_view;

    /** A mutable contiguous null-terminated sequence of char.  The sequence
        is assumed to be UTF-8 encoded, though it is possible to construct a
        sequence which is not.  Strongly exception safe. */
    struct text
    {
        using iterator = char *;
        using const_iterator = char const *;
        using reverse_iterator = detail::reverse_char_iterator;
        using const_reverse_iterator = detail::const_reverse_char_iterator;

        /** Default ctor.

            \post size() == 0 && capacity() == 0; begin(), end() delimit a
            valid, null-terminated empty string */
        text() noexcept : storage_(), size_(0), heap_(false) {}

        text(text const & t);

        text(text && rhs) noexcept : storage_(), size_(0), heap_(false)
        {
            swap(rhs);
        }

        /** Constructs a text from a text_view. */
        explicit text(text_view tv);

        /** Constructs a text from a repeated_text_view. */
        explicit text(repeated_text_view tv);

#ifdef BOOST_TEXT_DOXYGEN

        /** Constructs a text from a range of char.

            This function only participates in overload resolution if
            CharRange models the Char_range concept.

            \throw std::invalid_argument if the ends of the range are not
            valid UTF-8. */
        template<typename CharRange>
        explicit text(CharRange const & r);

        /** Constructs a text from a sequence of char.

            The sequence's UTF-8 encoding is not checked.  To check the
            encoding, use a converting iterator.

            This function only participates in overload resolution if Iter
            models the Char_iterator concept. */
        template<typename Iter>
        text(Iter first, Iter last);

#else

        template<typename CharRange>
        explicit text(
            CharRange const & r, detail::rng_alg_ret_t<int *, CharRange> = 0) :
            storage_(),
            size_(0),
            heap_(false)
        {
            insert(0, r);
        }

        template<typename Iter>
        text(Iter first, Iter last, detail::char_iter_ret_t<void *, Iter> = 0) :
            storage_(),
            size_(0),
            heap_(false)
        {
            insert(0, first, last);
        }

#endif

        ~text()
        {
            if (heap_)
                storage_.heap_.~heap_t();
        }

        text & operator=(text const & t);

        text & operator=(text && rhs) noexcept
        {
            swap(rhs);
            return *this;
        }

#ifdef BOOST_TEXT_DOXYGEN

        /** Assignment from a range of char.

            This function only participates in overload resolution if
            CharRange models the Char_range concept.

            \throw std::invalid_argument if the ends of the range are not
            valid UTF-8. */
        template<typename CharRange>
        text & operator=(CharRange const & r);

#else

        template<typename CharRange>
        auto operator=(CharRange const & r)
            -> detail::rng_alg_ret_t<text &, CharRange>;

#endif

        /** Assignment from a text_view. */
        text & operator=(text_view tv);

        /** Assignment from a repeated_text_view. */
        text & operator=(repeated_text_view tv);

        iterator begin() noexcept { return ptr(); }
        iterator end() noexcept { return ptr() + size_; }

        const_iterator begin() const noexcept { return ptr(); }
        const_iterator end() const noexcept { return ptr() + size_; }

        const_iterator cbegin() const noexcept { return begin(); }
        const_iterator cend() const noexcept { return end(); }

        reverse_iterator rbegin() noexcept
        {
            return reverse_iterator(end() - 1);
        }
        reverse_iterator rend() noexcept
        {
            return reverse_iterator(begin() - 1);
        }

        const_reverse_iterator rbegin() const noexcept
        {
            return const_reverse_iterator(end());
        }
        const_reverse_iterator rend() const noexcept
        {
            return const_reverse_iterator(begin());
        }

        const_reverse_iterator crbegin() const noexcept
        {
            return const_reverse_iterator(end());
        }
        const_reverse_iterator crend() const noexcept
        {
            return const_reverse_iterator(begin());
        }

        /** Returns true if size() == 0, false otherwise.

            An empty text is still a valid null-terminated empty string. */
        bool empty() const noexcept { return size_ == 0; }

        /** Returns the number of characters controlled by *this, not
            including the null terminator. */
        int size() const noexcept { return size_; }

        /** Returns the number of bytes of storage currently in use by *this.

            Even if the capcity is 0, the text is still a valid
            null-terminated empty string. */
        int capacity() const noexcept { return cap() - 1; }

        /** Returns the i-th char of *this (not a reference).

            \pre 0 <= i && i < size() */
        char operator[](int i) const noexcept
        {
#ifndef BOOST_TEXT_TESTING
            assert(0 <= i && i < size_);
#endif
            return ptr()[i];
        }

        /** Returns a substring of *this, taken from the range of chars at
            offsets [lo, hi).  If either of lo or hi is a negative value x, x
            is taken to be an offset from the end, and so x + size() is used
            instead.

            These preconditions apply to the values used after size() is added
            to any negative arguments.

            \pre 0 <= lo && lo <= size()
            \pre 0 <= hi && lhi <= size()
            \pre lo <= hi
            \throw std::invalid_argument if the ends of the string are not
            valid UTF-8. */
        text_view operator()(int lo, int hi) const;

        /** Returns a substring of *this, taken from the first cut chars when
            cut => 0, or the last -cut chars when cut < 0.

            \pre 0 <= cut && cut <= size() || 0 <= -cut && -cut <= size()
            \throw std::invalid_argument if the ends of the string are not
            valid UTF-8. */
        text_view operator()(int cut) const;

        /** Returns the maximum size a text can have. */
        int max_size() const noexcept { return INT_MAX / 2; }

        /** Lexicographical compare.  Returns a value < 0 when *this is
            lexicographically less than rhs, 0 if *this == rhs, and a value >
            0 if *this is lexicographically greater than rhs. */
        int compare(text_view rhs) const noexcept;

        bool operator==(text_view rhs) const noexcept;
        bool operator!=(text_view rhs) const noexcept;
        bool operator<(text_view rhs) const noexcept;
        bool operator<=(text_view rhs) const noexcept;
        bool operator>(text_view rhs) const noexcept;
        bool operator>=(text_view rhs) const noexcept;

        /** Clear.

            \post size() == 0 && capacity() == 0; begin(), end() delimit a
            valid, null-terminated empty string */
        void clear() noexcept
        {
            size_ = 0;
            ptr()[0] = '\0';
        }

        /** Returns a reference to the i-th char of *this.

            No check is made (or could be made) to check that writes through
            the returned reference do not break UTF-8 encoding. */
        char & operator[](int i) noexcept
        {
#ifndef BOOST_TEXT_TESTING
            assert(0 <= 0 && i < size_);
#endif
            return ptr()[i];
        }

        /** Inserts the sequence of char from tv into *this starting at offset
            at.

            \throw std::invalid_argument if insertion at offset at would break
            UTF-8 encoding. */
        text & insert(int at, text_view tv);

        /** Inserts the sequence of char from rtv into *this starting at
            offset at.

            \throw std::invalid_argument if insertion at offset at would break
            UTF-8 encoding. */
        text & insert(int at, repeated_text_view rtv);

#ifdef BOOST_TEXT_DOXYGEN

        /** Inserts the char range r into *this starting at offset at.

            This function only participates in overload resolution if
            CharRange models the Char_range concept.

            \throw std::invalid_argument if insertion at offset at would break
            UTF-8 encoding, or if the ends of the range are not valid
            UTF-8. */
        template<typename CharRange>
        text & insert(int at, CharRange const & r);

        /** Inserts the char sequence [first, last) into *this starting at
            offset at.

            This function only participates in overload resolution if Iter
            models the Char_iterator concept.

            The inserted sequence's UTF-8 encoding is not checked.  To check
            the encoding, use a converting iterator.

            \throw std::invalid_argument if insertion at offset at would break
            UTF-8 encoding. */
        template<typename Iter>
        text & insert(int at, Iter first, Iter last);

        /** Inserts the char sequence [first, last) into *this starting at
            position at.

            This function only participates in overload resolution if Iter
            models the Char_iterator concept.

            No check is made to determine if insertion at position at would
            break UTF-8 encoding, and the inserted sequence's UTF-8 encoding
            is not checked.  To check the inserted sequence's encoding, use a
            converting iterator. */
        template<typename Iter>
        text & insert(iterator at, Iter first, Iter last);

#else

        template<typename CharRange>
        auto insert(int at, CharRange const & r)
            -> detail::rng_alg_ret_t<text &, CharRange>;

        template<typename Iter>
        auto insert(int at, Iter first, Iter last)
            -> detail::char_iter_ret_t<text &, Iter>
        {
            assert(0 <= at && at <= size_);

            if (first == last)
                return *this;

            return insert_iter_impl(at, first, last);
        }

        template<typename Iter>
        auto insert(iterator at, Iter first, Iter last)
            -> detail::char_iter_ret_t<text &, Iter>
        {
            assert(begin() <= at && at <= end());

            if (first == last)
                return *this;

            return insert_iter_impl(at - begin(), first, last);
        }

#endif

        /** Erases the portion of *this delimited by tv.

            \pre !std::less(tv.begin(), begin()) && !std::less(end(),
            tv.end()) */
        text & erase(text_view tv) noexcept;

        /** Erases the portion of *this delimited by [first, last).

            No check is made to determine whether erasing [first, last) breaks
            UTF-8 encoding.

            \pre first <= last */
        text & erase(iterator first, iterator last) noexcept
        {
            assert(first <= last);
            assert(begin() <= first && last <= end());

            std::copy(last, end(), first);
            size_ -= last - first;
            ptr()[size_] = '\0';

            return *this;
        }

        /** Replaces the portion of *this delimited by old_substr with the
            sequence of char from new_substr.

            \pre !std::less(old_substr.begin(), begin()) && !std::less(end(),
            old_substr.end()) */
        text & replace(text_view old_substr, text_view new_substr);

        /** Replaces the portion of *this delimited by old_substr with the
            sequence of char from new_substr.

            \pre !std::less(old_substr.begin(), begin()) && !std::less(end(),
            old_substr.end()) */
        text & replace(text_view old_substr, repeated_text_view new_substr);

#ifdef BOOST_TEXT_DOXYGEN

        /** Replaces the portion of *this delimited by old_substr with the
            char range r.

            This function only participates in overload resolution if
            CharRange models the Char_range concept.

            \throw std::invalid_argument if the ends of the range are not
            valid UTF-8.
            \pre !std::less(old_substr.begin(), begin()) && !std::less(end(),
            old_substr.end()) */
        template<typename CharRange>
        text & replace(text_view old_substr, CharRange const & r);

        /** Replaces the portion of *this delimited by old_substr with the
            char sequence [first, last).

            This function only participates in overload resolution if Iter
            models the Char_iterator concept.

            The inserted sequence's UTF-8 encoding is not checked.  To check
            the encoding, use a converting iterator.

            \pre !std::less(old_substr.begin(), begin()) && !std::less(end(),
            old_substr.end()) */
        template<typename Iter>
        text & replace(text_view old_substr, Iter first, Iter last);

        /** Replaces the portion of *this delimited by [old_first, old_last)
            with the char sequence [new_first, new_last).

            This function only participates in overload resolution if Iter
            models the Char_iterator concept.

            No check is made to determine if removing [old_first, old_last)
            would break UTF-8 encoding, and the inserted sequence's UTF-8
            encoding is not checked.  To check the inserted sequence's
            encoding, use a converting iterator.

           \pre old_first <= old_last */
        template<typename Iter>
        text & replace(
            iterator old_first,
            iterator old_last,
            Iter new_first,
            Iter new_last);

#else

        template<typename CharRange>
        auto replace(text_view old_substr, CharRange const & r)
            -> detail::rng_alg_ret_t<text &, CharRange>;

        template<typename Iter>
        auto replace(text_view old_substr, Iter first, Iter last)
            -> detail::char_iter_ret_t<text &, Iter>;

        // TODO: Perf test replace(Iter) against insert(Iter), and replace the
        // insert(Iter) implementation if that is warranted.
        template<typename Iter>
        auto replace(
            iterator old_first,
            iterator old_last,
            Iter new_first,
            Iter new_last) -> detail::char_iter_ret_t<text &, Iter>
        {
            assert(begin() <= old_first && old_last <= end());
            assert(old_first <= old_last);

            char stack_buf[1024];
            std::list<heap_t> heap_bufs;
            int const chars_pushed = read_iters(
                stack_buf, sizeof(stack_buf), heap_bufs, new_first, new_last);
            int const stack_buf_bytes =
                (std::min)(chars_pushed, (int)sizeof(stack_buf));

            int const delta = chars_pushed - (old_last - old_first);
            int const available = capacity() - size_;
            if (available < delta) {
                heap_t new_data = get_new_data(delta - available);
                char * buf = new_data.data_.get();
                buf = std::copy(begin(), old_first, buf);
                buf = copy_bufs(stack_buf, stack_buf_bytes, heap_bufs, buf);
                std::copy(old_last, end(), buf);
                set_heap(std::move(new_data));
            } else {
                if (0 < delta)
                    std::copy_backward(old_last, end(), end() + delta);
                else if (delta < 0)
                    std::copy(old_last, end(), old_last + delta);
                copy_bufs(stack_buf, stack_buf_bytes, heap_bufs, old_first);
            }

            size_ += delta;
            ptr()[size_] = '\0';

            return *this;
        }

#endif

        /** Changes the size of *this to new_size.  Truncates if new_size <
            size(), and appends new_size - size() repetitions of c it size() <
            new_size.

            \throw std::invalid_argument if truncating to new_size would break
            UTF-8 encoding.
            \post size() == new_size */
        void resize(int new_size, char c)
        {
            assert(0 <= new_size);

            if (c & 0x80)
                throw std::invalid_argument(
                    "Given character is not a valid UTF-8 1-character code "
                    "point");

            int const prev_size = size_;
            int const delta = new_size - prev_size;
            if (!delta)
                return;

            int const available = capacity() - size_;
            if (available < delta) {
                heap_t new_data = get_new_data(delta - available);
                std::copy(begin(), begin() + prev_size, new_data.data_.get());
                set_heap(std::move(new_data));
            }

            size_ = new_size;

            if (0 < delta)
                std::fill(begin() + prev_size, end(), c);

            ptr()[size_] = '\0';
        }

        /** Reserves storage enough for a string of at least new_size
            bytes.

            \post capacity() >= new_size + 1 */
        void reserve(int new_size)
        {
            assert(0 <= new_size);
            int const new_cap = new_size + 1;
            if (new_cap <= cap())
                return;
            std::unique_ptr<char[]> new_ptr(new char[new_cap]);
            heap_t new_data{std::move(new_ptr), new_cap};
            *std::copy(cbegin(), cend(), new_data.data_.get()) = '\0';
            set_heap(std::move(new_data));
        }

        /** Reduces storage used by *this to just the amount necessary to
            contain size() chars.

            \post capacity() == 0 || capacity() == size() + 1 */
        void shrink_to_fit()
        {
            auto const c = cap();
            if (c == 0 || c == size_ + 1)
                return;
            std::unique_ptr<char[]> new_ptr(new char[size_ + 1]);
            heap_t new_data{std::move(new_ptr), size_ + 1};
            *std::copy(cbegin(), cend(), new_data.data_.get()) = '\0';
            set_heap(std::move(new_data));
        }

        /** Swaps *this with rhs. */
        void swap(text & rhs) noexcept
        {
            if (heap_ && rhs.heap_)
                std::swap(storage_.heap_, rhs.storage_.heap_);
            else if (!heap_ && !rhs.heap_)
                std::swap(storage_.local_, rhs.storage_.local_);
            else if (heap_)
                swap_local_and_heap(rhs.storage_, storage_);
            else
                swap_local_and_heap(storage_, rhs.storage_);

            {
                int tmp = size_;
                size_ = rhs.size_;
                rhs.size_ = tmp;
            }
            {
                bool tmp = heap_;
                heap_ = rhs.heap_;
                rhs.heap_ = tmp;
            }
        }

        /** Appends c_str to *this. */
        text & operator+=(char const * c_str);

        /** Appends tv to *this. */
        text & operator+=(text_view tv);

        /** Appends rtv to *this. */
        text & operator+=(repeated_text_view rtv);

        /** Appends r to *this. */
        text & operator+=(unencoded_rope r);

        /** Appends r to *this. */
        text & operator+=(unencoded_rope_view rv);

#ifdef BOOST_TEXT_DOXYGEN

        /** Appends the char range r to *this.

            This function only participates in overload resolution if
            CharRange models the Char_range concept.

            \throw std::invalid_argument if the ends of the range are not
            valid UTF-8. */
        template<typename CharRange>
        text & operator+=(CharRange const & r);

#else

        template<typename CharRange>
        auto operator+=(CharRange const & r)
            -> detail::rng_alg_ret_t<text &, CharRange>;

#endif

        /** Stream inserter; performs unformatted output. */
        friend std::ostream & operator<<(std::ostream & os, text const & t)
        {
            return os.write(t.begin(), t.size());
        }

#ifndef BOOST_TEXT_DOXYGEN

    private:
        struct heap_t
        {
            std::unique_ptr<char[]> data_;
            int cap_;
        };

        struct local_t
        {
            std::array<char, sizeof(heap_t)> buf_;
        };

        union storage_t
        {
            storage_t() { local_ = local_t(); }

            ~storage_t() {}

            heap_t heap_;
            local_t local_;
        };

        bool self_reference(text_view tv) const;

        int grow_cap(int min_new_cap) const
        {
            assert(0 < min_new_cap);
            int retval = cap();
            while (retval < min_new_cap) {
                retval = retval / 2 * 3;
            }
            int const rem = (retval + 16) % 16;
            retval += 16 - rem;
            return retval;
        }

        heap_t get_new_data(int resize_amount)
        {
            int const new_cap =
                0 < resize_amount ? grow_cap(cap() + resize_amount) : cap();
            return heap_t{std::unique_ptr<char[]>(new char[new_cap]), new_cap};
        }

        void push_char(char c)
        {
            int const available = capacity() - size_;
            if (available < 1) {
                heap_t new_data = get_new_data(1 - available);
                std::copy(cbegin(), cend(), new_data.data_.get());
                set_heap(std::move(new_data));
            }
            ptr()[size_] = c;
            ++size_;
        }

        template<typename Iter>
        auto insert_iter_impl(int at, Iter first, Iter last)
            -> detail::char_iter_ret_t<text &, Iter>
        {
            auto const initial_size = size_;
            try {
                while (first != last) {
                    push_char(*first);
                    ++first;
                }
            } catch (std::bad_alloc const &) {
                ptr()[size_] = '\0';
                throw;
            }

            std::rotate(begin() + at, begin() + initial_size, end());
            ptr()[size_] = '\0';

            return *this;
        }

        template<typename Iter>
        struct buf_ptr_iterator
        {
            char * buf_;
            Iter it_;
        };

        template<typename Iter>
        buf_ptr_iterator<Iter>
        fill_buf(char * buf, int size, Iter first, Iter last)
        {
            char * const buf_end = buf + size;
            while (first != last && buf != buf_end) {
                *buf = *first;
                ++buf;
                ++first;
            }
            return {buf, first};
        }

        char * copy_bufs(
            char * buf, int size, std::list<heap_t> const & bufs, char * it)
        {
            it = std::copy_n(buf, size, it);
            for (heap_t const & h : bufs) {
                it = std::copy_n(h.data_.get(), h.cap_, it);
            }
            return it;
        }

        template<typename Iter>
        int read_iters(
            char * buf,
            int size,
            std::list<heap_t> & bufs,
            Iter first,
            Iter last)
        {
            buf_ptr_iterator<Iter> buf_first = fill_buf(buf, size, first, last);

            int chars_pushed = buf_first.buf_ - buf;
            int buf_size = size;

            while (buf_first.it_ != last) {
                buf_size *= 2;
                bufs.push_back(heap_t());
                heap_t & temp = bufs.back();
                temp.data_.reset(new char[buf_size]);
                temp.cap_ = buf_size;
                buf_first =
                    fill_buf(temp.data_.get(), buf_size, buf_first.it_, last);
                temp.cap_ = buf_first.buf_ - temp.data_.get();
                chars_pushed += temp.cap_;
            }

            return chars_pushed;
        }

        storage_t storage_;
        int size_ : 31;
        bool heap_ : 1;

        char * ptr() noexcept
        {
            return heap_ ? storage_.heap_.data_.get()
                         : &storage_.local_.buf_[0];
        }

        char const * ptr() const noexcept
        {
            return heap_ ? storage_.heap_.data_.get()
                         : &storage_.local_.buf_[0];
        }

        int cap() const noexcept
        {
            return heap_ ? storage_.heap_.cap_ : sizeof(heap_t);
        }

        void set_heap(heap_t && heap)
        {
            if (!heap_) {
                new (&storage_.heap_) heap_t();
                heap_ = true;
            }
            storage_.heap_ = std::move(heap);
        }

        void swap_local_and_heap(storage_t & local, storage_t & heap)
        {
            auto const tmp = local.local_.buf_;
            new (&local.heap_) heap_t();
            std::swap(local.heap_, heap.heap_);
            heap.local_ = local_t();
            heap.local_.buf_ = tmp;
        }

#endif // Doxygen
    };

    inline text::iterator begin(text & t) noexcept { return t.begin(); }
    inline text::iterator end(text & t) noexcept { return t.end(); }
    inline text::const_iterator begin(text const & t) noexcept
    {
        return t.begin();
    }
    inline text::const_iterator end(text const & t) noexcept { return t.end(); }
    inline text::const_iterator cbegin(text const & t) noexcept
    {
        return t.cbegin();
    }
    inline text::const_iterator cend(text const & t) noexcept
    {
        return t.cend();
    }

    inline text::reverse_iterator rbegin(text & t) noexcept
    {
        return t.rbegin();
    }
    inline text::reverse_iterator rend(text & t) noexcept { return t.rend(); }
    inline text::const_reverse_iterator rbegin(text const & t) noexcept
    {
        return t.rbegin();
    }
    inline text::const_reverse_iterator rend(text const & t) noexcept
    {
        return t.rend();
    }
    inline text::const_reverse_iterator crbegin(text const & t) noexcept
    {
        return t.crbegin();
    }
    inline text::const_reverse_iterator crend(text const & t) noexcept
    {
        return t.crend();
    }

}}

#include <boost/text/repeated_text_view.hpp>

namespace boost { namespace text {

    namespace literals {

        /** Creates a text from a char string literal.

            \throw std::invalid_argument if the ends of the string are not
            valid UTF-8. */
        inline text operator"" _t(char const * str, std::size_t len)
        {
            assert(len < INT_MAX);
            return text(text_view(str, len));
        }
    }

#ifndef BOOST_TEXT_DOXYGEN

    inline text::text(text const & t) : storage_(), size_(0), heap_(false)
    {
        insert(0, text_view(t.begin(), t.size()));
    }

    inline text::text(text_view tv) : storage_(), size_(0), heap_(false)
    {
        insert(0, tv);
    }

    inline text::text(repeated_text_view rtv) :
        storage_(),
        size_(0),
        heap_(false)
    {
        insert(0, rtv);
    }

    inline text & text::operator=(text const & t)
    {
        if (t.size() <= size()) {
            clear();
            insert(0, text_view(t.begin(), t.size()));
        } else {
            text tmp(t);
            swap(tmp);
        }
        return *this;
    }

    template<typename CharRange>
    auto text::operator=(CharRange const & r)
        -> detail::rng_alg_ret_t<text &, CharRange>
    {
        return *this = text_view(r);
    }

    inline text & text::operator=(text_view tv)
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

    inline text & text::operator=(repeated_text_view rtv)
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

    inline text_view text::operator()(int lo, int hi) const
    {
        return text_view(*this)(lo, hi);
    }

    inline text_view text::operator()(int cut) const
    {
        return text_view(*this)(cut);
    }

    inline int text::compare(text_view rhs) const noexcept
    {
        return detail::compare_impl(begin(), end(), rhs.begin(), rhs.end());
    }

    inline bool text::operator==(text_view rhs) const noexcept
    {
        return compare(rhs) == 0;
    }

    inline bool text::operator!=(text_view rhs) const noexcept
    {
        return compare(rhs) != 0;
    }

    inline bool text::operator<(text_view rhs) const noexcept
    {
        return compare(rhs) < 0;
    }

    inline bool text::operator<=(text_view rhs) const noexcept
    {
        return compare(rhs) <= 0;
    }

    inline bool text::operator>(text_view rhs) const noexcept
    {
        return compare(rhs) > 0;
    }

    inline bool text::operator>=(text_view rhs) const noexcept
    {
        return compare(rhs) >= 0;
    }

    template<typename CharRange>
    auto text::insert(int at, CharRange const & r)
        -> detail::rng_alg_ret_t<text &, CharRange>
    {
        return insert(at, text_view(r));
    }

    inline text & text::insert(int at, text_view tv)
    {
        assert(0 <= at && at <= size_);
        assert(0 <= tv.size());

        bool const tv_null_terminated = !tv.empty() && tv.end()[-1] == '\0';
        if (tv_null_terminated)
            tv = tv(0, -1);

        int const delta = tv.size();
        if (!delta)
            return *this;

        bool const late_self_ref =
            self_reference(tv) && at < tv.end() - begin();
        int const available = capacity() - size_;
        if (late_self_ref || available < delta) {
            heap_t new_data = get_new_data(delta - available);
            char * buf = new_data.data_.get();
            buf = std::copy(cbegin(), cbegin() + at, buf);
            buf = std::copy(tv.begin(), tv.end(), buf);
            buf = std::copy(cbegin() + at, cend(), buf);
            set_heap(std::move(new_data));
        } else {
            std::copy_backward(cbegin() + at, cend(), end() + delta);
            char * buf = begin() + at;
            std::copy(tv.begin(), tv.end(), buf);
        }

        size_ += delta;
        ptr()[size_] = '\0';

        return *this;
    }

    inline text & text::insert(int at, repeated_text_view rtv)
    {
        assert(0 <= at && at <= size_);
        assert(0 <= rtv.size());

        bool const rtv_null_terminated =
            !rtv.view().empty() && rtv.view().end()[-1] == '\0';
        if (rtv_null_terminated)
            rtv = repeat(rtv.view()(0, -1), rtv.count());

        int const delta = rtv.size();
        if (!delta)
            return *this;

        bool const late_self_ref =
            self_reference(rtv.view()) && at < rtv.view().end() - begin();
        int const available = capacity() - size_;
        if (late_self_ref || available < delta) {
            heap_t new_data = get_new_data(delta - available);
            char * buf = new_data.data_.get();
            buf = std::copy(cbegin(), cbegin() + at, buf);
            for (int i = 0; i < rtv.count(); ++i) {
                buf = std::copy(rtv.view().begin(), rtv.view().end(), buf);
            }
            std::copy(cbegin() + at, cend(), buf);
            set_heap(std::move(new_data));
        } else {
            std::copy_backward(cbegin() + at, cend(), end() + delta);
            char * buf = begin() + at;
            for (int i = 0; i < rtv.count(); ++i) {
                buf = std::copy(rtv.view().begin(), rtv.view().end(), buf);
            }
        }

        size_ += delta;
        ptr()[size_] = '\0';

        return *this;
    }

    inline text & text::erase(text_view tv) noexcept
    {
        assert(0 <= tv.size());

        bool const tv_null_terminated = !tv.empty() && tv.end()[-1] == '\0';
        if (tv_null_terminated)
            tv = tv(0, -1);

        assert(self_reference(tv));

        char * first = const_cast<char *>(tv.begin());
        return erase(first, first + tv.size());
    }

    template<typename CharRange>
    auto text::replace(text_view old_substr, CharRange const & r)
        -> detail::rng_alg_ret_t<text &, CharRange>
    {
        return replace(old_substr, text_view(r));
    }

    inline text & text::replace(text_view old_substr, text_view new_substr)
    {
        assert(0 <= old_substr.size());
        assert(0 <= new_substr.size());

        bool const old_substr_null_terminated =
            !old_substr.empty() && old_substr.end()[-1] == '\0';
        if (old_substr_null_terminated)
            old_substr = old_substr(0, -1);

        assert(self_reference(old_substr));

        bool const new_substr_null_terminated =
            !new_substr.empty() && new_substr.end()[-1] == '\0';
        if (new_substr_null_terminated)
            new_substr = new_substr(0, -1);

        assert(begin() <= old_substr.begin() && old_substr.end() <= end());

        bool const late_self_ref =
            self_reference(new_substr) && old_substr.begin() < new_substr.end();
        int const delta = new_substr.size() - old_substr.size();
        int const available = capacity() - size_;
        if (late_self_ref || available < delta) {
            heap_t new_data = get_new_data(delta - available);
            char * buf = new_data.data_.get();
            buf = std::copy(cbegin(), old_substr.begin(), buf);
            buf = std::copy(new_substr.begin(), new_substr.end(), buf);
            std::copy(old_substr.end(), cend(), buf);
            set_heap(std::move(new_data));
        } else {
            if (0 < delta) {
                std::copy_backward(old_substr.end(), cend(), end() + delta);
            } else if (delta < 0) {
                std::copy(
                    old_substr.end(),
                    cend(),
                    const_cast<char *>(old_substr.end()) + delta);
            }
            char * buf = const_cast<char *>(old_substr.begin());
            std::copy(new_substr.begin(), new_substr.end(), buf);
        }

        size_ += delta;
        ptr()[size_] = '\0';

        return *this;
    }

    inline text &
    text::replace(text_view old_substr, repeated_text_view new_substr)
    {
        assert(0 <= old_substr.size());
        assert(0 <= new_substr.size());

        bool const old_substr_null_terminated =
            !old_substr.empty() && old_substr.end()[-1] == '\0';
        if (old_substr_null_terminated)
            old_substr = old_substr(0, -1);

        assert(self_reference(old_substr));

        bool const new_substr_null_terminated =
            !new_substr.view().empty() && new_substr.view().end()[-1] == '\0';
        if (new_substr_null_terminated)
            new_substr = repeat(new_substr.view()(0, -1), new_substr.count());

        assert(begin() <= old_substr.begin() && old_substr.end() <= end());

        bool const late_self_ref = self_reference(new_substr.view()) &&
                                   old_substr.begin() < new_substr.view().end();
        int const delta = new_substr.size() - old_substr.size();
        int const available = capacity() - size_;
        if (late_self_ref || available < delta) {
            heap_t new_data = get_new_data(delta - available);
            char * buf = new_data.data_.get();
            buf = std::copy(cbegin(), old_substr.begin(), buf);
            for (int i = 0; i < new_substr.count(); ++i) {
                buf = std::copy(
                    new_substr.view().begin(), new_substr.view().end(), buf);
            }
            std::copy(old_substr.end(), cend(), buf);
            set_heap(std::move(new_data));
        } else {
            if (0 < delta) {
                std::copy_backward(old_substr.end(), cend(), end() + delta);
            } else if (delta < 0) {
                std::copy(
                    old_substr.end(),
                    cend(),
                    const_cast<char *>(old_substr.end()) + delta);
            }
            char * buf = const_cast<char *>(old_substr.begin());
            for (int i = 0; i < new_substr.count(); ++i) {
                buf = std::copy(
                    new_substr.view().begin(), new_substr.view().end(), buf);
            }
        }

        size_ += delta;
        ptr()[size_] = '\0';

        return *this;
    }

    template<typename Iter>
    auto text::replace(text_view old_substr, Iter first, Iter last)
        -> detail::char_iter_ret_t<text &, Iter>
    {
        assert(0 <= old_substr.size());

        bool const old_substr_null_terminated =
            !old_substr.empty() && old_substr.end()[-1] == '\0';
        if (old_substr_null_terminated)
            old_substr = old_substr(0, -1);

        assert(self_reference(old_substr));

        char * old_first = const_cast<char *>(old_substr.begin());
        return replace(old_first, old_first + old_substr.size(), first, last);
    }

    inline text & text::operator+=(char const * c_str)
    {
        return insert(size(), c_str);
    }

    inline text & text::operator+=(text_view tv) { return insert(size(), tv); }

    inline text & text::operator+=(repeated_text_view rtv)
    {
        assert(0 <= rtv.size());
        reserve(size() + rtv.size());
        for (std::ptrdiff_t i = 0; i < rtv.count(); ++i) {
            insert(size(), rtv.view());
        }
        return *this;
    }

    template<typename CharRange>
    auto text::operator+=(CharRange const & r)
        -> detail::rng_alg_ret_t<text &, CharRange>
    {
        return insert(size(), text_view(r));
    }

    inline bool text::self_reference(text_view tv) const
    {
        using less_t = std::less<char const *>;
        less_t less;
        return !less(tv.begin(), begin()) && !less(end(), tv.end());
    }

#endif // Doxygen

    inline bool operator==(char const * lhs, text const & rhs) noexcept
    {
        return detail::compare_impl(
                   lhs, lhs + strlen(lhs), rhs.begin(), rhs.end()) == 0;
    }

    inline bool operator!=(char const * lhs, text const & rhs) noexcept
    {
        return detail::compare_impl(
                   lhs, lhs + strlen(lhs), rhs.begin(), rhs.end()) != 0;
    }

    inline bool operator<(char const * lhs, text const & rhs) noexcept
    {
        return detail::compare_impl(
                   lhs, lhs + strlen(lhs), rhs.begin(), rhs.end()) < 0;
    }

    inline bool operator<=(char const * lhs, text const & rhs) noexcept
    {
        return detail::compare_impl(
                   lhs, lhs + strlen(lhs), rhs.begin(), rhs.end()) <= 0;
    }

    inline bool operator>(char const * lhs, text const & rhs) noexcept
    {
        return detail::compare_impl(
                   lhs, lhs + strlen(lhs), rhs.begin(), rhs.end()) > 0;
    }

    inline bool operator>=(char const * lhs, text const & rhs) noexcept
    {
        return detail::compare_impl(
                   lhs, lhs + strlen(lhs), rhs.begin(), rhs.end()) >= 0;
    }


    /** Creates a new text object that is the concatenation of t and t2. */
    inline text operator+(text t, text const & t2) { return t += t2; }

    /** Creates a new text object that is the concatenation of t and tv. */
    inline text operator+(text t, text_view tv) { return t += tv; }

    /** Creates a new text object that is the concatenation of tv and t. */
    inline text operator+(text_view tv, text const & t)
    {
        return (text() += tv) += t;
    }

    /** Creates a new text object that is the concatenation of t and rtv. */
    inline text operator+(text t, repeated_text_view rtv) { return t += rtv; }

    /** Creates a new text object that is the concatenation of rtv and t. */
    inline text operator+(repeated_text_view rtv, text const & t)
    {
        return (text() += rtv) += t;
    }

#ifdef BOOST_TEXT_DOXYGEN

    /** Creates a new text object that is the concatenation of t and r.

        This function only participates in overload resolution if CharRange
        models the Char_range concept.

        \throw std::invalid_argument if the ends of the range are not valid
        UTF-8. */
    template<typename CharRange>
    text operator+(text t, CharRange const & r);

    /** Creates a new text object that is the concatenation of r and t.

        This function only participates in overload resolution if CharRange
        models the Char_range concept.

        \throw std::invalid_argument if the ends of the range are not valid
        UTF-8. */
    template<typename CharRange>
    text operator+(CharRange const & r, text const & t);

#else

    template<typename CharRange>
    auto operator+(text t, CharRange const & r)
        -> detail::rng_alg_ret_t<text, CharRange>
    {
        return t += r;
    }

    template<typename CharRange>
    auto operator+(CharRange const & r, text const & t)
        -> detail::rng_alg_ret_t<text, CharRange>
    {
        return (text() += r) += t;
    }

#endif

}}

#endif
