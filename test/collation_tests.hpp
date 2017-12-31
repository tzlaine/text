#ifndef BOOST_TEXT_TEST_COLLATION_TESTS_HPP
#define BOOST_TEXT_TEST_COLLATION_TESTS_HPP

#include <boost/text/collate.hpp>
#include <boost/text/normalize.hpp>

#include <iomanip>


struct ce_dumper
{
    template<int N>
    ce_dumper(uint32_t const (&ces)[N]) : first_(ces), last_(ces + N)
    {}

    ce_dumper(std::vector<uint32_t> const & ces) :
        first_(&*ces.begin()),
        last_(&*ces.end())
    {}

    friend std::ostream & operator<<(std::ostream & os, ce_dumper d)
    {
        os << std::hex;
        for (uint32_t const * it = d.first_; it != d.last_; ++it) {
            if (it != d.first_)
                os << ", ";
            os << "0x" << std::setfill('0') << std::setw(4) << *it;
        }
        os << "\n" << std::dec;
        return os;
    }

private:
    uint32_t const * first_;
    uint32_t const * last_;
};

// TODO: Implement something like this in collate.hpp.
std::vector<uint32_t> collate_for_tests(
    uint32_t const * first_,
    uint32_t const * last_,
    boost::text::variable_weighting weighting,
    boost::text::collation_strength strength)
{
    boost::text::string str = boost::text::to_string(first_, last_);
#if 0 // TODO: turn this back on when/if we can get the FCC-form to work.
    boost::text::pseudonormalize_to_fcc(str);
#else
    boost::text::normalize_to_nfd(str);
#endif

    boost::container::static_vector<uint32_t, 1024> buf;
    boost::text::utf32_range as_utf32(str);
    std::copy(as_utf32.begin(), as_utf32.end(), std::back_inserter(buf));

    uint32_t * first = &*buf.begin();
    uint32_t * last = &*buf.end();

    boost::container::small_vector<boost::text::collation_element, 1024> ces;
    boost::text::detail::s2(first, last, weighting, ces);

    std::vector<uint32_t> retval;
    boost::text::detail::s3(
        ces,
        strength,
        boost::text::l2_weight_order::forward,
        first,
        last,
        last - first,
        retval);

    return retval;
}

#endif
