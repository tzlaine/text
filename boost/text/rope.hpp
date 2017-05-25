#ifndef BOOST_TEXT_ROPE_HPP
#define BOOST_TEXT_ROPE_HPP

#include <boost/text/text_view.hpp>
#include <boost/text/text.hpp>

#include <boost/variant.hpp>
#include <boost/variant/get.hpp>

#include <vector>


namespace boost { namespace text {

    struct rope
    {
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

        constexpr bool empty () const noexcept
        { return segments_.size() == 0; }

        constexpr std::ptrdiff_t size () const noexcept
        {
            std::ptrdiff_t retval = 0;
            for (std::size_t i = 0; i < segments_.size(); ++i) {
                switch (segments_[i].which()) {
                case 0: retval += get<text_view>(segments_[i]).size(); break;
                case 1: retval += get<rope_ptr_t>(segments_[i])->size(); break;
                default: assert(!"Unhandled case in rope.");
                }
            }
            return retval;
        }

        constexpr char front () const noexcept
        {
            assert(!empty());
            switch (segments_[0].which()) {
            case 0: return get<text_view>(segments_[0]).front();
            case 1: return get<rope_ptr_t>(segments_[0])->front();
            default: assert(!"Unhandled case in rope."); return '\0';
            }
        }

        constexpr char back () const noexcept
        {
            assert(!empty());
            auto const i = segments_.size() - 1;
            switch (segments_[i].which()) {
            case 0: return get<text_view>(segments_[i]).back();
            case 1: return get<rope_ptr_t>(segments_[i])->back();
            default: assert(!"Unhandled case in rope."); return '\0';
            }
        }

        constexpr char operator[] (std::ptrdiff_t n) const noexcept
        {
            assert(!empty());
            for (std::size_t i = 0; i < segments_.size(); ++i) {
                segment_t const & segment = segments_[i];
                switch (segment.which()) {
                case 0: {
                    text_view const view = get<text_view>(segment);
                    std::ptrdiff_t const seg_size = view.size();
                    if (n < seg_size)
                        return view[n];
                    else
                        n -= seg_size;
                    break;
                }
                case 1: {
                    rope_ptr_t const & rope_ptr = get<rope_ptr_t>(segment);
                    std::ptrdiff_t const seg_size = rope_ptr->size();
                    if (n < seg_size)
                        return (*rope_ptr)[n];
                    else
                        n -= seg_size;
                    break;
                }
                default: assert(!"Unhandled case in rope.");
                }
            }
            return '\0';
        }

        constexpr std::ptrdiff_t max_size () const noexcept
        { return PTRDIFF_MAX; }

#if 0
        // TODO: operator<=> () const
        constexpr int compare (rope rhs) const noexcept
        { return detail::compare_impl(begin(), end(), rhs.begin(), rhs.end()); }

        constexpr bool operator== (rope rhs) const noexcept
        { return compare(rhs) == 0; }

        constexpr bool operator!= (rope rhs) const noexcept
        { return compare(rhs) != 0; }

        constexpr bool operator< (rope rhs) const noexcept
        { return compare(rhs) < 0; }

        constexpr bool operator<= (rope rhs) const noexcept
        { return compare(rhs) <= 0; }

        constexpr bool operator> (rope rhs) const noexcept
        { return compare(rhs) > 0; }

        constexpr bool operator>= (rope rhs) const noexcept
        { return compare(rhs) >= 0; }

        friend constexpr iterator begin (rope v) noexcept
        { return v.begin(); }
        friend constexpr iterator end (rope v) noexcept
        { return v.end(); }
        friend constexpr iterator cbegin (rope v) noexcept
        { return v.cbegin(); }
        friend constexpr iterator cend (rope v) noexcept
        { return v.cend(); }

        friend constexpr reverse_iterator rbegin (rope v) noexcept
        { return v.rbegin(); }
        friend constexpr reverse_iterator rend (rope v) noexcept
        { return v.rend(); }
        friend constexpr reverse_iterator crbegin (rope v) noexcept
        { return v.crbegin(); }
        friend constexpr reverse_iterator crend (rope v) noexcept
        { return v.crend(); }

        friend std::ostream & operator<< (std::ostream & os, rope view)
        { TODO; }
#endif

    private:
        using rope_ptr_t = std::shared_ptr<rope>;
        using segment_t = boost::variant<
            text_view,
// TODO            text,
            rope_ptr_t
        >;
        std::vector<segment_t> segments_;
    };

} }

#endif
