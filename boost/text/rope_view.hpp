#ifndef BOOST_TEXT_ROPE_VIEW_HPP
#define BOOST_TEXT_ROPE_VIEW_HPP

#include <boost/text/text_view.hpp>


namespace boost { namespace text {

    struct rope_view
    {
        constexpr rope_view () noexcept :
            size_ (0)
        {}

#if 0
        constexpr const_iterator begin () const noexcept { return data_; }
        constexpr const_iterator end () const noexcept { return data_ + size_; }

        constexpr const_iterator cbegin () const noexcept { return begin(); }
        constexpr const_iterator cend () const noexcept { return end(); }

        constexpr const_reverse_iterator rbegin () const noexcept { return reverse_iterator(end()); }
        constexpr const_reverse_iterator rend () const noexcept { return reverse_iterator(begin()); }

        constexpr const_reverse_iterator crbegin () const noexcept { return rbegin(); }
        constexpr const_reverse_iterator crend () const noexcept { return rend(); }
#endif

        constexpr empty () const noexcept
        { return size_ == 0; }

        constexpr std::ptrdiff_t size () const noexcept
        {
            std::ptrdiff_t retval = 0;
            for (int i = 0; i < size_; ++i) {
                switch (elements_[i].which_) {
                case elements::tv: retval += elements_[i].ref_.tv_.size(); break;
                case elements::rv: retval += elements_[i].ref_.rv_->size(); break;
                default: assert(!"Unhandled case in rope_view.");
                }
            }
            return retval;
        }

        constexpr char front () const noexcept
        {
            assert(!empty());
            switch (elements_[0].which_) {
            case elements::tv: return elements_[0].ref_.tv_.front();
            case elements::rv: return elements_[0].ref_.rv_->front();
            default: assert(!"Unhandled case in rope_view."); return '\0';
            }
        }

        constexpr char back () const noexcept
        {
            assert(!empty());
            int const i = size_ - 1;
            switch (elements_[i].which_) {
            case elements::tv: return elements_[i].ref_.tv_.back();
            case elements::rv: return elements_[i].ref_.rv_->back();
            default: assert(!"Unhandled case in rope_view."); return '\0';
            }
        }

        constexpr char operator[] (std::ptrdiff_t n) const noexcept
        {
            assert(!empty());
            for (int i = 0; i < size_; ++i) {
                switch (elements_[i].which_) {
                case elements::tv: {
                    std::ptrdiff_t const elem_size = elements_[i].ref_.tv_.size();
                    if (n < elem_size)
                        return elements_[i].ref_.tv_[n];
                    else
                        n -= elem_size;
                    break;
                }
                case elements::rv: {
                    std::ptrdiff_t const elem_size = elements_[i].ref_.rv_.size();
                    if (n < elem_size)
                        return elements_[i].ref_.rv_[n];
                    else
                        n -= elem_size;
                    break;
                }
                default: assert(!"Unhandled case in rope_view.");
                }
            }
        }

        constexpr std::ptrdiff_t max_size () const noexcept
        { return PTRDIFF_MAX; }

#if 0
        constexpr int compare (rope_view rhs) const noexcept
        { return detail::compare_impl(begin(), end(), rhs.begin(), rhs.end()); }

        constexpr bool operator== (rope_view rhs) const noexcept
        { return compare(rhs) == 0; }

        constexpr bool operator!= (rope_view rhs) const noexcept
        { return compare(rhs) != 0; }

        constexpr bool operator< (rope_view rhs) const noexcept
        { return compare(rhs) < 0; }

        constexpr bool operator<= (rope_view rhs) const noexcept
        { return compare(rhs) <= 0; }

        constexpr bool operator> (rope_view rhs) const noexcept
        { return compare(rhs) > 0; }

        constexpr bool operator>= (rope_view rhs) const noexcept
        { return compare(rhs) >= 0; }

        friend constexpr iterator begin (rope_view v) noexcept
        { return v.begin(); }
        friend constexpr iterator end (rope_view v) noexcept
        { return v.end(); }
        friend constexpr iterator cbegin (rope_view v) noexcept
        { return v.cbegin(); }
        friend constexpr iterator cend (rope_view v) noexcept
        { return v.cend(); }

        friend constexpr reverse_iterator rbegin (rope_view v) noexcept
        { return v.rbegin(); }
        friend constexpr reverse_iterator rend (rope_view v) noexcept
        { return v.rend(); }
        friend constexpr reverse_iterator crbegin (rope_view v) noexcept
        { return v.crbegin(); }
        friend constexpr reverse_iterator crend (rope_view v) noexcept
        { return v.crend(); }

        friend std::ostream & operator<< (std::ostream & os, rope_view view)
        { TODO; }
#endif

    private:
        struct element
        {
            enum which { tv, rv };

            element () : ref_ (), rope_ref_ (false) {}

            union text_ref
            {
                text_view tv_;
                rope_view const * rv_;
            };

            text_ref ref_;
            which which_;
        };

        element elements_[8];
        int size_;
    };

} }

#endif
