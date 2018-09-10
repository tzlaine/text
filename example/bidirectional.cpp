#include <boost/text/bidirectional.hpp>
#include <boost/text/text.hpp>

#include <iostream>


int main ()
{

{
//[ bidi_simple
// This is the Arabic text as it appears in Google translate, already
// right-to-left.  This is how we expect it to appear below, after the
// bidirectional algorithm processes it.
boost::text::text const RTL_order_arabic = "مرحبا ، عالم ثنائي الاتجاه";

boost::text::text const memory_order_text =
    u8"When I type \"Hello, bidirectional world\" into Google translate "
    u8"English->Arabic, it produces \"هاجتالا يئانث ملاع ، ابحرم\".  I have no "
    u8"idea if it's correct.\n";

/* Prints:
When I type "Hello, bidirectional world" into Google translate English->Arabic, it produces "مرحبا ، عالم ثنائي الاتجاه".  I have no idea if it's correct.
*/
boost::text::text bidirectional_text;
for (auto range : boost::text::bidirectional_subranges(memory_order_text)) {
    for (auto grapheme : range) {
        std::cout << grapheme;
        bidirectional_text.insert(bidirectional_text.end(), grapheme);
    }
}

// TODO: Broken!
std::cout << bidirectional_text;
//]
}

}
