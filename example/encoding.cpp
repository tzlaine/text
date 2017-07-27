#include <boost/text/text.hpp>

boost::text::text string_from_terminal ()
{ return boost::text::text(""); } // TODO

int main ()
{

//[ start_with_encoding

    // We can start with a literal or other user-verified encoded string.
    boost::text::text_view tv = "всем привет!";

    // Or we can explicitly run over the entire string and check its encoding.
    boost::text::text t = boost::text::checked_encoding(string_from_terminal());
//]
    (void)t;

//[ encoding_affecting_operations
    try {
        // If we try to slice the encoding, we get an error.
        boost::text::text_view slice_0 = tv(0, 1);
        assert(!"We should never evaluate this assert.");
    } catch (...) {}

    // But as long as we slice along code point boundaries, everything works.
    boost::text::text_view slice_1 = tv(0, 2);
//]
    (void)slice_1;
}
