// Copyright (C) 2020 T. Zachary Laine
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
#ifndef BOOST_TEXT_TEST_COLLATION_TESTS_HPP
#define BOOST_TEXT_TEST_COLLATION_TESTS_HPP

#include <boost/text/collate.hpp>
#include <boost/text/normalize_string.hpp>

#include <iomanip>


struct ce_dumper
{
    template<int N>
    ce_dumper(uint32_t const (&ces)[N]) : first_(ces), last_(ces + N)
    {}

    template<typename Range>
    ce_dumper(Range const & r) : first_(&*r.begin()), last_(&*r.end())
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

boost::text::collation_table const & table()
{
    static auto const retval = boost::text::default_collation_table();
    return retval;
}

std::vector<uint32_t> collate_for_tests(
    uint32_t const * first_,
    uint32_t const * last_,
    boost::text::variable_weighting weighting,
    boost::text::collation_strength strength)
{
    boost::text::string str = boost::text::to_string(first_, last_);
    boost::text::normalize<boost::text::nf::fcc>(str);

    boost::container::static_vector<uint32_t, 1024> buf;
    auto const r = boost::text::as_utf32(str);
    std::copy(r.begin(), r.end(), std::back_inserter(buf));

    uint32_t * first = &*buf.begin();
    uint32_t * last = &*buf.end();

    boost::container::small_vector<boost::text::detail::collation_element, 1024>
        ces((last - first) * 10);
    auto ces_end = table().copy_collation_elements(
        first,
        last,
        ces.begin(),
        boost::text::collation_strength::primary,
        boost::text::case_first::off,
        boost::text::case_level::off,
        weighting);
    ces.resize(ces_end - ces.begin());

    std::vector<uint32_t> retval;
    boost::text::detail::s3(
        ces.begin(),
        ces.end(),
        ces.size(),
        strength,
        boost::text::case_first::off,
        boost::text::case_level::off,
        boost::text::l2_weight_order::forward,
        first,
        last,
        last - first,
        retval);

    return retval;
}

#endif
