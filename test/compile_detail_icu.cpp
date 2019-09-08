#include <boost/text/normalize.hpp>
#include <boost/text/string_view.hpp>

#include <vector>


using namespace boost;


#if BOOST_TEXT_HAS_ICU

void make_byte_sink_()
{
    {
        char const * cstr;
        char * str;
        using cstr_bytes_sink = decltype(text::detail::make_byte_sink(cstr));
        using str_bytes_sink = decltype(text::detail::make_byte_sink(str));

        static_assert(
            std::is_same<
                cstr_bytes_sink,
                text::detail::out_iter_sink<char const *, char const *>>::value,
            "");

        static_assert(
            std::is_same<
                str_bytes_sink,
                text::detail::out_iter_sink<char *, char *>>::value,
            "");
    }
    {
        using char_it = text::utf_32_to_8_insert_iterator<std::vector<char>>;
        std::vector<char> vec;
        char_it ci(vec, vec.begin());
        using char_bytes_sink = decltype(text::detail::make_byte_sink(ci));

        static_assert(
            std::is_same<
                char_bytes_sink,
                text::detail::out_iter_sink<
                    std::insert_iterator<std::vector<char>>,
                    char_it>>::value,
            "");
    }
    {
        using char_it =
            text::utf_32_to_8_back_insert_iterator<std::vector<char>>;
        std::vector<char> vec;
        char_it ci(vec);
        using char_bytes_sink = decltype(text::detail::make_byte_sink(ci));

        static_assert(
            std::is_same<
                char_bytes_sink,
                text::detail::out_iter_sink<
                    std::back_insert_iterator<std::vector<char>>,
                    char_it>>::value,
            "");
    }
}

void char_ptr_()
{
    // positive tests
    static_assert(text::detail::char_ptr<char const *>::value, "");
    static_assert(text::detail::char_ptr<char *>::value, "");

    // negative tests
    static_assert(!text::detail::char_ptr<char const *&>::value, "");
    static_assert(!text::detail::char_ptr<char *&>::value, "");
    static_assert(!text::detail::char_ptr<char const>::value, "");
    static_assert(!text::detail::char_ptr<char>::value, "");
    static_assert(
        !text::detail::char_ptr<std::vector<char>::iterator>::value, "");
    static_assert(
        !text::detail::char_ptr<std::vector<char>::const_iterator>::value, "");
}

void char_ptr_or_base_char_ptr_()
{
    // positive tests
    static_assert(
        text::detail::char_ptr_or_base_char_ptr<char const *>::value, "");
    static_assert(text::detail::char_ptr_or_base_char_ptr<char *>::value, "");
    static_assert(
        text::detail::char_ptr_or_base_char_ptr<
            text::utf_8_to_32_iterator<char const *>>::value,
        "");
    static_assert(
        text::detail::char_ptr_or_base_char_ptr<
            text::utf_8_to_32_iterator<char *>>::value,
        "");
    static_assert(
        text::detail::char_ptr_or_base_char_ptr<
            text::utf_8_to_32_iterator<char const *, text::null_sentinel>>::
            value,
        "");
    static_assert(
        text::detail::char_ptr_or_base_char_ptr<
            text::utf_8_to_32_iterator<char *, text::null_sentinel>>::value,
        "");

    // negative tests
    static_assert(
        !text::detail::char_ptr_or_base_char_ptr<char const *&>::value, "");
    static_assert(!text::detail::char_ptr_or_base_char_ptr<char *&>::value, "");
    static_assert(
        !text::detail::char_ptr_or_base_char_ptr<char const>::value, "");
    static_assert(!text::detail::char_ptr_or_base_char_ptr<char>::value, "");
    static_assert(
        !text::detail::char_ptr_or_base_char_ptr<
            std::vector<char>::iterator>::value,
        "");
    static_assert(
        !text::detail::char_ptr_or_base_char_ptr<
            std::vector<char>::const_iterator>::value,
        "");
}

void char_out_iter_()
{
    // positive tests
    {
        static_assert(text::detail::char_out_iter<char *>::value, "");
        static_assert(
            text::detail::char_out_iter<std::vector<char>::iterator>::value,
            "");
    }
    {
        static_assert(
            text::detail::char_out_iter<
                text::utf_32_to_8_insert_iterator<std::vector<char>>>::value,
            "");
        static_assert(
            text::detail::char_out_iter<
                std::insert_iterator<std::vector<char>>>::value,
            "");
    }
    {
        static_assert(
            text::detail::char_out_iter<text::utf_32_to_8_back_insert_iterator<
                std::vector<char>>>::value,
            "");
        static_assert(
            text::detail::char_out_iter<
                std::back_insert_iterator<std::vector<char>>>::value,
            "");
    }

    // negative tests
    {
        static_assert(!text::detail::char_out_iter<int>::value, "");
        static_assert(!text::detail::char_out_iter<char const *>::value, "");
        static_assert(
            !text::detail::char_out_iter<
                std::vector<char>::const_iterator>::value,
            "");
        static_assert(
            !text::detail::char_out_iter<std::back_insert_iterator<
                boost::container::small_vector<unsigned int, 256>>>::value,
            "");
    }
}

void fast_path()
{
    std::vector<char> normalized;
    std::string file_contents;
    auto r = boost::text::make_to_utf32_range(text::string_view(file_contents));
    auto out = text::utf_32_to_8_back_inserter(normalized);

    static_assert(
        text::detail::char_ptr_or_base_char_ptr<decltype(r.begin())>::value,
        "");
    static_assert(
        text::detail::char_ptr_or_base_char_ptr<decltype(r.begin())>::value,
        "");
    static_assert(text::detail::char_out_iter<decltype(out)>::value, "");

    static_assert(
        text::detail::icu_utf8_inout_fast_path<
            decltype(r.begin()),
            decltype(r.end()),
            decltype(out)>::value,
        "");
}

#endif
