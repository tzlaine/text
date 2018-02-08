#ifndef BOOST_TEXT_TABLE_SERIALIZATION_HPP
#define BOOST_TEXT_TABLE_SERIALIZATION_HPP

#include <boost/text/collation_table.hpp>

#include <boost/filesystem/path.hpp>

#include <exception>


namespace boost { namespace text {

    /** TODO */
    void
    save_table(collation_table const & table, filesystem::path const & path);

    /** TODO */
    collation_table load_table(filesystem::path const & path);

    /** TODO */
    // TODO: Common base for this and parse_error.
    // TODO: Use BOOST_THROW or whatever.
    struct serialization_error : std::exception
    {
        serialization_error(string_view msg) : msg_(msg) {}
        char const * what() const noexcept { return msg_.begin(); }

    private:
        string msg_;
    };

}}

#endif
