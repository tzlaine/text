#ifndef BOOST_TEXT_STRING_HPP
#define BOOST_TEXT_STRING_HPP

#include <boost/text/config.hpp>

#include <boost/text/detail/algorithm.hpp>
#include <boost/text/detail/iterator.hpp>
#include <boost/text/detail/utility.hpp>

#include <boost/algorithm/cxx14/equal.hpp>

#include <algorithm>
#include <array>
#include <list>
#include <memory>

#include <cassert>


namespace boost { namespace text {

    struct string_view;
    struct repeated_string_view;
    struct unencoded_rope;
    struct unencoded_rope_view;

    // TODO: Document that string is not quite a SequenceContainer.  Missing:
    // {const_,}reference; {difference,size}_type; X(n, t) (constructing a
    // sequence of n copies of t); a.insert(p,n,t) (insert n copies of t at
    // p); a.insert(p, il) (insert initializer_list); assign() (because we
    // have range-based assignemnt).

    /** A mutable contiguous null-terminated sequence of char.  Strongly
        exception safe. */
    struct string
    {
        using value_type = char;
        using iterator = char *;
        using const_iterator = char const *;
        using reverse_iterator = detail::reverse_char_iterator;
        using const_reverse_iterator = detail::const_reverse_char_iterator;

        /** Default ctor.

            \post size() == 0 && capacity() == 0; begin(), end() delimit a
            valid, null-terminated empty string */
        string() noexcept : storage_(), size_(0), heap_(false) {}

        string(string const & s);

        string(string && rhs) noexcept : storage_(), size_(0), heap_(false)
        {
            swap(rhs);
        }

        /** Constructs a string from a null-terminated string. */
        string(char const * c_str);

        /** Constructs a string from a null-terminated string. */
        template<int N>
        string(char (&c_str)[N]);

        /** Constructs a string from a string_view. */
        explicit string(string_view sv);

        /** Constructs a string from a repeated_string_view. */
        explicit string(repeated_string_view rsv);

#ifdef BOOST_TEXT_DOXYGEN

        /** Constructs a string from a range of char.

            This function only participates in overload resolution if
            CharRange models the CharRange concept. */
        template<typename CharRange>
        explicit string(CharRange const & r);

        /** Constructs a string from a sequence of char.

            This function only participates in overload resolution if
            CharIter models the CharIter concept. */
        template<typename CharIter>
        string(CharIter first, CharIter last);

        /** Constructs a string from a range of graphemes over an underlying
            range of char.

            This function only participates in overload resolution if
            GraphemeRange models the GraphemeRange concept. */
        template<typename GraphemeRange>
        explicit string(GraphemeRange const & r);

#else

        template<typename CharRange>
        explicit string(
            CharRange const & r, detail::rng_alg_ret_t<int *, CharRange> = 0) :
            storage_(),
            size_(0),
            heap_(false)
        {
            insert(0, r);
        }

        template<typename CharIter>
        string(
            CharIter first,
            CharIter last,
            detail::char_iter_ret_t<void *, CharIter> = 0) :
            storage_(),
            size_(0),
            heap_(false)
        {
            insert(0, first, last);
        }

        template<typename GraphemeRange>
        explicit string(
            GraphemeRange const & r,
            detail::graph_rng_alg_ret_t<int *, GraphemeRange> = 0) :
            storage_(),
            size_(0),
            heap_(false)
        {
            insert(0, r);
        }

#endif

        ~string()
        {
            if (heap_)
                storage_.heap_.~heap_t();
        }

        string & operator=(char const * c_str);

        template<int N>
        string & operator=(char (&c_str)[N]);

        string & operator=(string const & s);

        string & operator=(string && rhs) noexcept
        {
            swap(rhs);
            return *this;
        }

#ifdef BOOST_TEXT_DOXYGEN

        /** Assignment from a range of char.

            This function only participates in overload resolution if
            CharRange models the CharRange concept. */
        template<typename CharRange>
        string & operator=(CharRange const & r);

        /** Assignment from a range of graphemes over an underlying range of
            char.

            This function only participates in overload resolution if
            GraphemeRange models the GraphemeRange concept. */
        template<typename GraphemeRange>
        string & operator=(GraphemeRange const & r);

#else

        template<typename CharRange>
        auto operator=(CharRange const & r)
            -> detail::rng_alg_ret_t<string &, CharRange>;

        template<typename GraphemeRange>
        auto operator=(GraphemeRange const & r)
            -> detail::graph_rng_alg_ret_t<string &, GraphemeRange>;

#endif

        /** Assignment from a string_view. */
        string & operator=(string_view sv);

        /** Assignment from a repeated_string_view. */
        string & operator=(repeated_string_view rsv);

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

            An empty string is still a valid null-terminated empty string. */
        bool empty() const noexcept { return size_ == 0; }

        /** Returns the number of characters controlled by *this, not
            including the null terminator. */
        int size() const noexcept { return size_; }

        /** Returns the number of bytes of storage currently in use by *this.

            Even if the capcity is 0, the string is still a valid
            null-terminated empty string. */
        int capacity() const noexcept { return cap() - 1; }

        // TODO: Document this in the top-level docs.
        /** Returns the char (not a reference) of *this at index i, or the
            char at index -i when i < 0.

            \pre 0 <= i && i <= size() || 0 <= -i && -i <= size()  */
        char operator[](int i) const noexcept
        {
            if (i < 0)
                i += size_;
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
            \pre lo <= hi */
        string_view operator()(int lo, int hi) const;

        /** Returns a substring of *this, taken from the first cut chars when
            cut => 0, or the last -cut chars when cut < 0.

            \pre 0 <= cut && cut <= size() || 0 <= -cut && -cut <= size()  */
        string_view operator()(int cut) const;

        /** Returns the maximum size a string can have. */
        int max_size() const noexcept { return INT_MAX / 2; }

        /** Lexicographical compare.  Returns a value < 0 when *this is
            lexicographically less than rhs, 0 if *this == rhs, and a value >
            0 if *this is lexicographically greater than rhs. */
        int compare(string_view rhs) const noexcept;

        /** Clear.

            \post size() == 0 && capacity() == 0; begin(), end() delimit a
            valid, null-terminated empty string */
        void clear() noexcept
        {
            size_ = 0;
            ptr()[0] = '\0';
        }

        /** Returns a reference to the char at index i, or the
            char at index -i when i < 0.

            \pre 0 <= i && i <= size() || 0 <= -i && -i <= size()  */
        char & operator[](int i) noexcept
        {
            if (i < 0)
                i += size_;
#ifndef BOOST_TEXT_TESTING
            assert(0 <= i && i < size_);
#endif
            return ptr()[i];
        }

        /** Inserts c into *this at position at. */
        iterator insert(iterator at, char c);
        /** Inserts c into *this at offset at. */
        string & insert(int at, char c);

        /** Inserts the sequence of char from sv into *this starting at offset
            at. */
        string & insert(int at, string_view sv);
        /** Inserts the sequence of char from sv into *this starting at
            position at. */
        iterator insert(iterator at, string_view sv);

        /** Inserts the sequence of char from rsv into *this starting at
            offset at. */
        string & insert(int at, repeated_string_view rsv);
        /** Inserts the sequence of char from rsv into *this starting at
            position at. */
        iterator insert(iterator at, repeated_string_view rsv);

#ifdef BOOST_TEXT_DOXYGEN

        /** Inserts the char range r into *this starting at offset at.

            This function only participates in overload resolution if
            CharRange models the CharRange concept. */
        template<typename CharRange>
        string & insert(int at, CharRange const & r);

        /** Inserts the char range r into *this starting at position at.

            This function only participates in overload resolution if
            CharRange models the CharRange concept. */
        template<typename CharRange>
        iterator insert(iterator at, CharRange const & r);

        /** Inserts the char sequence [first, last) into *this starting at
            offset at.

            This function only participates in overload resolution if CharIter
            models the CharIter concept. */
        template<typename CharIter>
        string & insert(int at, CharIter first, CharIter last);

        /** Inserts the char sequence [first, last) into *this starting at
            position at.

            This function only participates in overload resolution if CharIter
            models the CharIter concept. */
        template<typename CharIter>
        iterator insert(iterator at, CharIter first, CharIter last);

        /** Inserts the underlying range of char from the given range of
            graphemes into *this starting at offset at.

            This function only participates in overload resolution if
            GraphemeRange models the GraphemeRange concept. */
        template<typename GraphemeRange>
        string & insert(int at, GraphemeRange const & r);

        /** Inserts the underlying range of char from the given range of
            graphemes into *this starting at position at.

            This function only participates in overload resolution if
            GraphemeRange models the GraphemeRange concept. */
        template<typename GraphemeRange>
        iterator insert(iterator at, GraphemeRange const & r);

#else

        template<typename CharRange>
        auto insert(int at, CharRange const & r)
            -> detail::rng_alg_ret_t<string &, CharRange>;

        template<typename CharRange>
        auto insert(iterator at, CharRange const & r)
            -> detail::rng_alg_ret_t<iterator, CharRange>;

        template<typename CharIter>
        auto insert(int at, CharIter first, CharIter last)
            -> detail::char_iter_ret_t<string &, CharIter>
        {
            assert(0 <= at && at <= size_);

            if (first == last)
                return *this;

            insert_iter_impl(at, first, last);

            return *this;
        }

        template<typename CharIter>
        auto insert(iterator at, CharIter first, CharIter last)
            -> detail::char_iter_ret_t<iterator, CharIter>
        {
            assert(begin() <= at && at <= end());

            if (first == last)
                return at;

            return insert_iter_impl(at - begin(), first, last);
        }

        template<typename GraphemeRange>
        auto insert(int at, GraphemeRange const & r)
            -> detail::graph_rng_alg_ret_t<string &, GraphemeRange>;

        template<typename GraphemeRange>
        auto insert(iterator at, GraphemeRange const & r)
            -> detail::graph_rng_alg_ret_t<iterator, GraphemeRange>;

#endif

        /** Erases the portion of *this delimited by sv.

            \pre !std::less(tv.begin(), begin()) && !std::less(end(),
            tv.end()) */
        string & erase(string_view sv) noexcept;

        /** Erases the portion of *this delimited by [first, last).

            \pre first <= last */
        string & erase(iterator first, iterator last) noexcept
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
        string & replace(string_view old_substr, string_view new_substr);

        /** Replaces the portion of *this delimited by old_substr with the
            sequence of char from new_substr.

            \pre !std::less(old_substr.begin(), begin()) && !std::less(end(),
            old_substr.end()) */
        string &
        replace(string_view old_substr, repeated_string_view new_substr);

#ifdef BOOST_TEXT_DOXYGEN

        /** Replaces the portion of *this delimited by old_substr with the
            char range r.

            This function only participates in overload resolution if
            CharRange models the CharRange concept.

            \pre !std::less(old_substr.begin(), begin()) && !std::less(end(),
            old_substr.end()) */
        template<typename CharRange>
        string & replace(string_view old_substr, CharRange const & r);

        /** Replaces the portion of *this delimited by old_substr with the
            char sequence [first, last).

            This function only participates in overload resolution if CharIter
            models the CharIter concept.

            \pre !std::less(old_substr.begin(), begin()) && !std::less(end(),
            old_substr.end()) */
        template<typename CharIter>
        string & replace(string_view old_substr, CharIter first, CharIter last);

        /** Replaces the portion of *this delimited by [old_first, old_last)
            with the char sequence [new_first, new_last).

            This function only participates in overload resolution if CharIter
            models the CharIter concept.

           \pre old_first <= old_last */
        template<typename CharIter>
        string & replace(
            iterator old_first,
            iterator old_last,
            CharIter new_first,
            CharIter new_last);

#else

        template<typename CharRange>
        auto replace(string_view old_substr, CharRange const & r)
            -> detail::rng_alg_ret_t<string &, CharRange>;

        template<typename CharIter>
        auto replace(string_view old_substr, CharIter first, CharIter last)
            -> detail::char_iter_ret_t<string &, CharIter>;

        // TODO: Perf test replace(CharIter) against insert(CharIter), and
        // replace the insert(CharIter) implementation if that is warranted.
        template<typename CharIter>
        auto replace(
            iterator old_first,
            iterator old_last,
            CharIter new_first,
            CharIter new_last) -> detail::char_iter_ret_t<string &, CharIter>
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

            \post size() == new_size */
        void resize(int new_size, char c)
        {
            assert(0 <= new_size);

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
        void swap(string & rhs) noexcept
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

        /** Appends c to *this. */
        string & operator+=(char c);

        /** Appends c_str to *this. */
        string & operator+=(char const * c_str);

        /** Appends tv to *this. */
        string & operator+=(string_view sv);

        /** Appends rtv to *this. */
        string & operator+=(repeated_string_view rsv);

        /** Appends r to *this. */
        string & operator+=(unencoded_rope r);

        /** Appends r to *this. */
        string & operator+=(unencoded_rope_view rv);

#ifdef BOOST_TEXT_DOXYGEN

        /** Appends the char range r to *this.

            This function only participates in overload resolution if
            CharRange models the CharRange concept. */
        template<typename CharRange>
        string & operator+=(CharRange const & r);

#else

        template<typename CharRange>
        auto operator+=(CharRange const & r)
            -> detail::rng_alg_ret_t<string &, CharRange>;

#endif

        /** Stream inserter; performs unformatted output. */
        friend std::ostream & operator<<(std::ostream & os, string const & s)
        {
            return os.write(s.begin(), s.size());
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

        bool self_reference(string_view tv) const;

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

        template<typename CharIter>
        auto insert_iter_impl(int at, CharIter first, CharIter last)
            -> detail::char_iter_ret_t<iterator, CharIter>
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

            return begin() + at + (size_ - initial_size);
        }

        template<typename CharIter>
        struct buf_ptr_iterator
        {
            char * buf_;
            CharIter it_;
        };

        template<typename CharIter>
        buf_ptr_iterator<CharIter>
        fill_buf(char * buf, int size, CharIter first, CharIter last)
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

        template<typename CharIter>
        int read_iters(
            char * buf,
            int size,
            std::list<heap_t> & bufs,
            CharIter first,
            CharIter last)
        {
            buf_ptr_iterator<CharIter> buf_first =
                fill_buf(buf, size, first, last);

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

    inline string::iterator begin(string & s) noexcept { return s.begin(); }
    inline string::iterator end(string & s) noexcept { return s.end(); }
    inline string::const_iterator begin(string const & s) noexcept
    {
        return s.begin();
    }
    inline string::const_iterator end(string const & s) noexcept
    {
        return s.end();
    }
    inline string::const_iterator cbegin(string const & s) noexcept
    {
        return s.cbegin();
    }
    inline string::const_iterator cend(string const & s) noexcept
    {
        return s.cend();
    }

    inline string::reverse_iterator rbegin(string & s) noexcept
    {
        return s.rbegin();
    }
    inline string::reverse_iterator rend(string & s) noexcept
    {
        return s.rend();
    }
    inline string::const_reverse_iterator rbegin(string const & s) noexcept
    {
        return s.rbegin();
    }
    inline string::const_reverse_iterator rend(string const & s) noexcept
    {
        return s.rend();
    }
    inline string::const_reverse_iterator crbegin(string const & s) noexcept
    {
        return s.crbegin();
    }
    inline string::const_reverse_iterator crend(string const & s) noexcept
    {
        return s.crend();
    }

}}

#include <boost/text/repeated_string_view.hpp>

namespace boost { namespace text {

    namespace literals {

        /** Creates a string from a char string literal. */
        inline string operator"" _s(char const * str, std::size_t len)
        {
            assert(len < INT_MAX);
            return string(string_view(str, len));
        }
    }

#ifndef BOOST_TEXT_DOXYGEN

    inline string::string(string const & s) : storage_(), size_(0), heap_(false)
    {
        insert(0, string_view(s.begin(), s.size() + 1));
    }

    inline string::string(char const * c_str) :
        storage_(),
        size_(0),
        heap_(false)
    {
        insert(0, string_view(c_str));
    }

    template<int N>
    string::string(char (&c_str)[N]) : storage_(), size_(0), heap_(false)
    {
        insert(0, string_view(c_str, N - 1));
    }

    inline string::string(string_view sv) : storage_(), size_(0), heap_(false)
    {
        insert(0, sv);
    }

    inline string::string(repeated_string_view rsv) :
        storage_(),
        size_(0),
        heap_(false)
    {
        insert(0, rsv);
    }

    inline string & string::operator=(char const * c_str)
    {
        return *this = string_view(c_str);
    }

    template<int N>
    string & string::operator=(char (&c_str)[N])
    {
        return *this = string_view(c_str, N - 1);
    }

    inline string & string::operator=(string const & s)
    {
        if (s.size() <= size()) {
            clear();
            insert(0, string_view(s.begin(), s.size() + 1));
        } else {
            string tmp(s);
            swap(tmp);
        }
        return *this;
    }

    inline string & string::operator=(string_view sv)
    {
        bool const self_ref = self_reference(sv);
        if (self_ref) {
            erase(string_view(sv.end(), end() - sv.end()));
            erase(string_view(begin(), sv.begin() - begin()));
        } else if (sv.size() <= size()) {
            clear();
            insert(0, sv);
        } else {
            string tmp(sv);
            swap(tmp);
        }
        return *this;
    }

    template<typename CharRange>
    auto string::operator=(CharRange const & r)
        -> detail::rng_alg_ret_t<string &, CharRange>
    {
        using std::begin;
        using std::end;
        if (std::distance(begin(r), end(r)) <= size()) {
            clear();
            insert(0, r);
        } else {
            string tmp(r);
            swap(tmp);
        }
        return *this;
    }


    template<typename GraphemeRange>
    auto string::operator=(GraphemeRange const & r)
        -> detail::graph_rng_alg_ret_t<string &, GraphemeRange>
    {
        using std::begin;
        using std::end;
        auto const first = begin(r).base().base();
        auto const last = end(r).base().base();
        if (std::distance(first, last) <= size()) {
            clear();
            insert(0, r);
        } else {
            string tmp(r);
            swap(tmp);
        }
        return *this;
    }

    inline string & string::operator=(repeated_string_view rsv)
    {
        assert(0 <= rsv.size());
        bool const self_ref = self_reference(rsv.view());
        if (!self_ref && rsv.size() <= size()) {
            clear();
            insert(0, rsv);
        } else {
            string tmp(rsv);
            swap(tmp);
        }
        return *this;
    }

    inline string_view string::operator()(int lo, int hi) const
    {
        return string_view(*this)(lo, hi);
    }

    inline string_view string::operator()(int cut) const
    {
        return string_view(*this)(cut);
    }

    inline int string::compare(string_view rhs) const noexcept
    {
        return detail::compare_impl(begin(), end(), rhs.begin(), rhs.end());
    }

    inline string & string::insert(int at, char c)
    {
        char chars[2] = {c, 0};
        return insert(at, string_view(chars, 2));
    }

    inline string::iterator string::insert(iterator at, char c)
    {
        char chars[2] = {c, 0};
        return insert(at, string_view(chars, 2));
    }

    template<typename CharRange>
    auto string::insert(int at, CharRange const & r)
        -> detail::rng_alg_ret_t<string &, CharRange>
    {
        using std::begin;
        using std::end;
        return insert(at, begin(r), end(r));
    }

    template<typename CharRange>
    auto string::insert(iterator at, CharRange const & r)
        -> detail::rng_alg_ret_t<iterator, CharRange>
    {
        using std::begin;
        using std::end;
        return insert(at, begin(r), end(r));
    }

    inline string & string::insert(int at, string_view sv)
    {
        assert(0 <= at && at <= size_);
        assert(0 <= sv.size());

        bool const sv_null_terminated = !sv.empty() && sv.end()[-1] == '\0';
        if (sv_null_terminated)
            sv = sv(0, -1);

        int const delta = sv.size();
        if (!delta)
            return *this;

        bool const late_self_ref =
            self_reference(sv) && at < sv.end() - begin();
        int const available = capacity() - size_;
        if (late_self_ref || available < delta) {
            heap_t new_data = get_new_data(delta - available);
            char * buf = new_data.data_.get();
            buf = std::copy(cbegin(), cbegin() + at, buf);
            buf = std::copy(sv.begin(), sv.end(), buf);
            buf = std::copy(cbegin() + at, cend(), buf);
            set_heap(std::move(new_data));
        } else {
            std::copy_backward(cbegin() + at, cend(), end() + delta);
            char * buf = begin() + at;
            std::copy(sv.begin(), sv.end(), buf);
        }

        size_ += delta;
        ptr()[size_] = '\0';

        return *this;
    }

    inline string::iterator string::insert(iterator at, string_view sv)
    {
        auto const offset = at - begin();
        insert(at - begin(), sv);
        return begin() + offset;
    }

    inline string & string::insert(int at, repeated_string_view rsv)
    {
        assert(0 <= at && at <= size_);
        assert(0 <= rsv.size());

        bool const rsv_null_terminated =
            !rsv.view().empty() && rsv.view().end()[-1] == '\0';
        if (rsv_null_terminated)
            rsv = repeat(rsv.view()(0, -1), rsv.count());

        int const delta = rsv.size();
        if (!delta)
            return *this;

        bool const late_self_ref =
            self_reference(rsv.view()) && at < rsv.view().end() - begin();
        int const available = capacity() - size_;
        if (late_self_ref || available < delta) {
            heap_t new_data = get_new_data(delta - available);
            char * buf = new_data.data_.get();
            buf = std::copy(cbegin(), cbegin() + at, buf);
            for (int i = 0; i < rsv.count(); ++i) {
                buf = std::copy(rsv.view().begin(), rsv.view().end(), buf);
            }
            std::copy(cbegin() + at, cend(), buf);
            set_heap(std::move(new_data));
        } else {
            std::copy_backward(cbegin() + at, cend(), end() + delta);
            char * buf = begin() + at;
            for (int i = 0; i < rsv.count(); ++i) {
                buf = std::copy(rsv.view().begin(), rsv.view().end(), buf);
            }
        }

        size_ += delta;
        ptr()[size_] = '\0';

        return *this;
    }

    inline string::iterator
    string::insert(iterator at, repeated_string_view rsv)
    {
        auto const offset = at - begin();
        insert(at - begin(), rsv);
        return begin() + offset;
    }

    template<typename GraphemeRange>
    auto string::insert(int at, GraphemeRange const & r)
        -> detail::graph_rng_alg_ret_t<string &, GraphemeRange>
    {
        using std::begin;
        using std::end;
        return insert(at, begin(r).base().base(), end(r).base().base());
    }

    template<typename GraphemeRange>
    auto string::insert(iterator at, GraphemeRange const & r)
        -> detail::graph_rng_alg_ret_t<iterator, GraphemeRange>
    {
        using std::begin;
        using std::end;
        return insert(at, begin(r).base().base(), end(r).base().base());
    }

    inline string & string::erase(string_view sv) noexcept
    {
        assert(0 <= sv.size());

        bool const sv_null_terminated = !sv.empty() && sv.end()[-1] == '\0';
        if (sv_null_terminated)
            sv = sv(0, -1);

        assert(self_reference(sv));

        char * first = const_cast<char *>(sv.begin());
        return erase(first, first + sv.size());
    }

    template<typename CharRange>
    auto string::replace(string_view old_substr, CharRange const & r)
        -> detail::rng_alg_ret_t<string &, CharRange>
    {
        return replace(old_substr, string_view(r));
    }

    inline string &
    string::replace(string_view old_substr, string_view new_substr)
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

    inline string &
    string::replace(string_view old_substr, repeated_string_view new_substr)
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

    template<typename CharIter>
    auto string::replace(string_view old_substr, CharIter first, CharIter last)
        -> detail::char_iter_ret_t<string &, CharIter>
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

    inline string & string::operator+=(char c) { return insert(size(), c); }

    inline string & string::operator+=(char const * c_str)
    {
        return insert(size(), c_str);
    }

    inline string & string::operator+=(string_view sv)
    {
        return insert(size(), sv);
    }

    inline string & string::operator+=(repeated_string_view rsv)
    {
        assert(0 <= rsv.size());
        reserve(size() + rsv.size());
        for (std::ptrdiff_t i = 0; i < rsv.count(); ++i) {
            insert(size(), rsv.view());
        }
        return *this;
    }

    template<typename CharRange>
    auto string::operator+=(CharRange const & r)
        -> detail::rng_alg_ret_t<string &, CharRange>
    {
        return insert(size(), string_view(r));
    }

    inline bool string::self_reference(string_view sv) const
    {
        using less_t = std::less<char const *>;
        less_t less;
        return !less(sv.begin(), begin()) && !less(end(), sv.end());
    }

#endif // Doxygen

    inline bool operator==(char const * lhs, string const & rhs) noexcept
    {
        return rhs.compare(string_view(lhs)) == 0;
    }
    inline bool operator==(string const & lhs, char const * rhs) noexcept
    {
        return lhs.compare(string_view(rhs)) == 0;
    }

    inline bool operator!=(char const * lhs, string const & rhs) noexcept
    {
        return rhs.compare(string_view(lhs)) != 0;
    }
    inline bool operator!=(string const & lhs, char const * rhs) noexcept
    {
        return lhs.compare(string_view(rhs)) != 0;
    }

    inline bool operator<(char const * lhs, string const & rhs) noexcept
    {
        return rhs.compare(string_view(lhs)) > 0;
    }
    inline bool operator<(string const & lhs, char const * rhs) noexcept
    {
        return lhs.compare(string_view(rhs)) < 0;
    }

    inline bool operator<=(char const * lhs, string const & rhs) noexcept
    {
        return rhs.compare(string_view(lhs)) >= 0;
    }
    inline bool operator<=(string const & lhs, char const * rhs) noexcept
    {
        return lhs.compare(string_view(rhs)) <= 0;
    }

    inline bool operator>(char const * lhs, string const & rhs) noexcept
    {
        return rhs.compare(string_view(lhs)) < 0;
    }
    inline bool operator>(string const & lhs, char const * rhs) noexcept
    {
        return lhs.compare(string_view(rhs)) > 0;
    }

    inline bool operator>=(char const * lhs, string const & rhs) noexcept
    {
        return rhs.compare(string_view(lhs)) <= 0;
    }
    inline bool operator>=(string const & lhs, char const * rhs) noexcept
    {
        return lhs.compare(string_view(rhs)) >= 0;
    }

    template<typename CharRange>
    auto operator==(CharRange const & lhs, string const & rhs) noexcept
        -> detail::rng_alg_ret_t<bool, CharRange, unencoded_rope>
    {
        using std::begin;
        using std::end;
        return algorithm::equal(begin(lhs), end(lhs), rhs.begin(), rhs.end());
    }
    template<typename CharRange>
    auto operator==(string const & lhs, CharRange const & rhs) noexcept
        -> detail::rng_alg_ret_t<bool, CharRange, string>
    {
        using std::begin;
        using std::end;
        return algorithm::equal(lhs.begin(), lhs.end(), begin(rhs), end(rhs));
    }

    template<typename CharRange>
    auto operator!=(CharRange const & lhs, string const & rhs) noexcept
        -> detail::rng_alg_ret_t<bool, CharRange, unencoded_rope>
    {
        return !(lhs == rhs);
    }
    template<typename CharRange>
    auto operator!=(string const & lhs, CharRange const & rhs) noexcept
        -> detail::rng_alg_ret_t<bool, CharRange, string>
    {
        return rhs != lhs;
    }

    template<typename CharRange>
    auto operator<(CharRange const & lhs, string const & rhs) noexcept
        -> detail::rng_alg_ret_t<bool, CharRange, unencoded_rope>
    {
        using std::begin;
        using std::end;
        return detail::generalized_compare(
                   begin(lhs), end(lhs), rhs.begin(), rhs.end()) < 0;
    }
    template<typename CharRange>
    auto operator<(string const & lhs, CharRange const & rhs) noexcept
        -> detail::rng_alg_ret_t<bool, CharRange, string>
    {
        using std::begin;
        using std::end;
        return detail::generalized_compare(
                   lhs.begin(), lhs.end(), begin(rhs), end(rhs)) < 0;
    }

    template<typename CharRange>
    auto operator<=(CharRange const & lhs, string const & rhs) noexcept
        -> detail::rng_alg_ret_t<bool, CharRange, unencoded_rope>
    {
        return lhs < rhs || lhs == rhs;
    }
    template<typename CharRange>
    auto operator<=(string const & lhs, CharRange const & rhs) noexcept
        -> detail::rng_alg_ret_t<bool, CharRange, string>
    {
        return lhs < rhs || lhs == rhs;
    }

    template<typename CharRange>
    auto operator>(CharRange const & lhs, string const & rhs) noexcept
        -> detail::rng_alg_ret_t<bool, CharRange, unencoded_rope>
    {
        return rhs < lhs;
    }
    template<typename CharRange>
    auto operator>(string const & lhs, CharRange const & rhs) noexcept
        -> detail::rng_alg_ret_t<bool, CharRange, string>
    {
        return rhs < lhs;
    }

    template<typename CharRange>
    auto operator>=(CharRange const & lhs, string const & rhs) noexcept
        -> detail::rng_alg_ret_t<bool, CharRange, unencoded_rope>
    {
        return rhs <= lhs;
    }
    template<typename CharRange>
    auto operator>=(string const & lhs, CharRange const & rhs) noexcept
        -> detail::rng_alg_ret_t<bool, CharRange, string>
    {
        return rhs <= lhs;
    }

    /** Creates a new string object that is the concatenation of s and c. */
    inline string operator+(string s, char c) { return s += c; }

    /** Creates a new string object that is the concatenation of s and c_str. */
    template<int N>
    inline string operator+(string s, char const (&c_str)[N])
    {
        return s += string_view(c_str, N - 1);
    }

    /** Creates a new string object that is the concatenation of c_str and s. */
    template<int N>
    inline string operator+(char const (&c_str)[N], string s)
    {
        return s.insert(0, string_view(c_str, N - 1));
    }

    /** Creates a new string object that is the concatenation of s and c_str. */
    inline string operator+(string s, char const * c_str)
    {
        return s += string_view(c_str);
    }

    /** Creates a new string object that is the concatenation of c_str and s. */
    inline string operator+(char const * c_str, string s)
    {
        return s.insert(0, string_view(c_str));
    }

    /** Creates a new string object that is the concatenation of s and sv. */
    inline string operator+(string const & s, string_view sv)
    {
        return string(s) += sv;
    }

    /** Creates a new string object that is the concatenation of s and sv.
     */
    inline string operator+(string && s, string_view sv) { return s += sv; }

    /** Creates a new string object that is the concatenation of s and s2. */
    inline string operator+(string const & s, string const & s2)
    {
        return string(s) += s2;
    }

    /** Creates a new string object that is the concatenation of s and s2.
     */
    inline string operator+(string && s, string const & s2) { return s += s2; }

    /** Creates a new string object that is the concatenation of s and s2. */
    inline string operator+(string const & s, string && s2)
    {
        return s2.insert(0, s);
    }

    /** Creates a new string object that is the concatenation of s and s2.
     */
    inline string operator+(string && s, string && s2) { return s += s2; }

    /** Creates a new string object that is the concatenation of s and rsv.
     */
    inline string operator+(string const & s, repeated_string_view rsv)
    {
        return string(s) += rsv;
    }

    /** Creates a new string object that is the concatenation of s and rsv.
     */
    inline string operator+(string && s, repeated_string_view rsv)
    {
        return s += rsv;
    }

    /** Creates a new string object that is the concatenation of sv and s. */
    inline string operator+(string_view sv, string const & s)
    {
        return (string() += sv) += s;
    }

    /** Creates a new string object that is the concatenation of rsv and s. */
    inline string operator+(repeated_string_view rsv, string const & s)
    {
        return (string() += rsv) += s;
    }

#ifdef BOOST_TEXT_DOXYGEN

    /** Creates a new string object that is the concatenation of s and r.

        This function only participates in overload resolution if CharRange
        models the CharRange concept. */
    template<typename CharRange>
    string operator+(string s, CharRange const & r);

    /** Creates a new string object that is the concatenation of r and s.

        This function only participates in overload resolution if CharRange
        models the CharRange concept. */
    template<typename CharRange>
    string operator+(CharRange const & r, string const & s);

#else

    template<typename CharRange>
    auto operator+(string s, CharRange const & r)
        -> detail::rng_alg_ret_t<string, CharRange>
    {
        return s += r;
    }

    template<typename CharRange>
    auto operator+(CharRange const & r, string const & s)
        -> detail::rng_alg_ret_t<string, CharRange>
    {
        return (string() += r) += s;
    }

#endif

}}

#endif
