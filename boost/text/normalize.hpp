#ifndef BOOST_TEXT_NORMALIZE_HPP
#define BOOST_TEXT_NORMALIZE_HPP

#include <boost/text/normalization_data.hpp>
#include <boost/text/string.hpp>
#include <boost/text/utility.hpp>

#include <boost/container/static_vector.hpp>


namespace boost { namespace text {

    namespace detail {

        template<typename Iter>
        void order_canonically(Iter first, Iter last)
        {
            if (first == last)
                return;
            --last;
            while (first != last) {
                auto it = first;
                auto new_last = first;
                while (it != last) {
                    auto next = it;
                    ++next;
                    // TODO: Optimize to reduce these function calls.
                    auto const ccc_a = ccc(*it);
                    auto const ccc_b = ccc(*next);
                    if (0 < ccc_b && ccc_b < ccc_a) {
                        std::iter_swap(it, next);
                        new_last = it;
                    }
                    ++it;
                }
                last = new_last;
            }
        }

        template<std::size_t Capacity>
        void flush_buffer(
            container::static_vector<uint32_t, Capacity> & buffer, string & str)
        {
            order_canonically(buffer.begin(), buffer.end());
            str.insert(
                str.size(),
                utf8::make_from_utf32_iterator(buffer.begin()),
                utf8::make_from_utf32_iterator(buffer.end()));
            buffer.clear();
        }
    }

    inline void normalize_to_nfd(string & s)
    {
        string temp;
        utf32_range as_utf32(s);
        container::static_vector<uint32_t, 64> buffer;
        for (auto x : as_utf32) {
            auto const decomp = canonical_decompose(x);
            if (!ccc(decomp.storage_[0]))
                detail::flush_buffer(buffer, temp);
            buffer.insert(buffer.end(), decomp.begin(), decomp.end());
        }
        detail::flush_buffer(buffer, temp);
        if (temp.size() <= s.capacity())
            s = temp;
        else
            s.swap(temp);
    }

    inline void normalize_to_nfkd(string & s)
    {
        // TODO
    }

    inline void normalize_to_nfc(string & s)
    {
        // TODO
    }

    inline void normalize_to_nfkc(string & s)
    {
        // TODO
    }

}}

#endif
