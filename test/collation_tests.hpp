#ifndef BOOST_TEXT_TEST_COLLATION_TESTS_HPP
#define BOOST_TEXT_TEST_COLLATION_TESTS_HPP

#include <boost/text/collate.hpp>
#include <boost/text/normalize.hpp>

#include <boost/container/small_vector.hpp>

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
    uint32_t * first,
    uint32_t * last,
    boost::text::variable_weighting weighting)
{
    boost::text::string str = boost::text::to_string(first, last);
    boost::text::psuedonormalize_to_fcc(str);

    boost::container::static_vector<uint32_t, 1024> buf;
    boost::text::utf32_range as_utf32(str);
    std::copy(as_utf32.begin(), as_utf32.end(), std::back_inserter(buf));

    first = &*buf.begin(); 
    last = &*buf.end(); 

    std::vector<boost::text::compressed_collation_element> cces;
    boost::text::detail::s2(first, last, cces);

    std::vector<boost::text::collation_element> ces(cces.size());
    std::transform(
        cces.begin(),
        cces.end(),
        ces.begin(),
        [](boost::text::compressed_collation_element ce) {
            return boost::text::collation_element{ce.l1(), ce.l2(), ce.l3()};
        });

    boost::text::detail::s2_3(ces, weighting);

    std::vector<uint32_t> retval;

    // TODO: Provide an API for passing in scratch space so that l[1-4] are not
    // repeatedly realloacted.
    boost::container::small_vector<uint32_t, 256> l1;
    boost::container::small_vector<uint32_t, 256> l2;
    boost::container::small_vector<uint32_t, 256> l3;
    boost::container::small_vector<uint32_t, 256> l4;
    l1.reserve(ces.size());
    l2.reserve(ces.size());
    l3.reserve(ces.size());
    l4.reserve(ces.size());

    for (auto ce : ces) {
        if (ce.l1_)
            l1.push_back(ce.l1_);
        if (ce.l2_)
            l2.push_back(ce.l2_);
        if (ce.l3_)
            l3.push_back(ce.l3_);
        if (ce.l4_)
            l4.push_back(ce.l4_);
    }

    retval.resize(l1.size() + l2.size() + l3.size() + l4.size() + 3);
    auto it = retval.begin();
    it = std::copy(l1.begin(), l1.end(), it);
    *it++ = 0x000;
    it = std::copy(l2.begin(), l2.end(), it);
    *it++ = 0x000;
    it = std::copy(l3.begin(), l3.end(), it);
    *it++ = 0x000;
    it = std::copy(l4.begin(), l4.end(), it);
    assert(it == retval.end());

    return retval;
}

#endif
