// Warning! This file is autogenerated.
#ifndef BOOST_TEXT_DATA_SK_HPP
#define BOOST_TEXT_DATA_SK_HPP

#include <boost/text/string_view.hpp>


namespace boost { namespace text { namespace data { namespace sk {

inline string_view search_collation_tailoring()
{
    return string_view(
u8R"(  
  
[normalization on]
[suppressContractions [เ-ไ ເ-ໄ ꪵ ꪶ ꪹ ꪻ ꪼ\u19B5-\u19B7\u19BA]]
&'='<'≠'
&ا
  <<<ﺎ<<<ﺍ
  <<آ
  <<<ﺂ<<<ﺁ
  <<أ
  <<<ﺄ<<<ﺃ
  <<إ
  <<<ﺈ<<<ﺇ
&و
  <<<ۥ
  <<<ﻮ<<<ﻭ
  <<ؤ
  <<<ﺆ<<<ﺅ
&ي
  <<<ۦ
  <<<ﻳ<<<ﻴ<<<ﻲ<<<ﻱ
  <<ئ
  <<<ﺋ<<<ﺌ<<<ﺊ<<<ﺉ
  <<ى
  <<<ﯨ<<<ﯩ
  <<<ﻰ<<<ﻯ
&ه
  <<<ﻫ<<<ﻬ<<<ﻪ<<<ﻩ
  <<ة
  <<<ﺔ<<<ﺓ
&[last primary ignorable]<<׳
  <<״
  <<ـ
  <<ฺ
&ᄀ
  =ᆨ
&ᄀᄀ
  =ᄁ=ᆩ
&ᄀᄉ
  =ᆪ
&ᄂ
  =ᆫ
&ᄂᄌ
  =ᆬ
&ᄂᄒ
  =ᆭ
&ᄃ
  =ᆮ
&ᄃᄃ
  =ᄄ
&ᄅ
  =ᆯ
&ᄅᄀ
  =ᆰ
&ᄅᄆ
  =ᆱ
&ᄅᄇ
  =ᆲ
&ᄅᄉ
  =ᆳ
&ᄅᄐ
  =ᆴ
&ᄅᄑ
  =ᆵ
&ᄅᄒ
  =ᆶ
&ᄆ
  =ᆷ
&ᄇ
  =ᆸ
&ᄇᄇ
  =ᄈ
&ᄇᄉ
  =ᆹ
&ᄉ
  =ᆺ
&ᄉᄉ
  =ᄊ=ᆻ
&ᄋ
  =ᆼ
&ᄌ
  =ᆽ
&ᄌᄌ
  =ᄍ
&ᄎ
  =ᆾ
&ᄏ
  =ᆿ
&ᄐ
  =ᇀ
&ᄑ
  =ᇁ
&ᄒ
  =ᇂ
&ᅡᅵ
  =ᅢ
&ᅣᅵ
  =ᅤ
&ᅥᅵ
  =ᅦ
&ᅧᅵ
  =ᅨ
&ᅩᅡ
  =ᅪ
&ᅩᅡᅵ
  =ᅫ
&ᅩᅵ
  =ᅬ
&ᅮᅥ
  =ᅯ
&ᅮᅥᅵ
  =ᅰ
&ᅮᅵ
  =ᅱ
&ᅳᅵ
  =ᅴ
  
&A<á<<<Á<ä<<<Ä
&C<č<<<Č
&D<ď<<<Ď
&E<é<<<É<ě<<<Ě
&I<í<<<Í
&L<ĺ<<<Ĺ<ľ<<<Ľ
&N<ň<<<Ň
&O<ó<<<Ó<ô<<<Ô
&R<ŕ<<<Ŕ<ř<<<Ř
&S<š<<<Š
&T<ť<<<Ť
&U<ú<<<Ú<ů<<<Ů
&Y<ý<<<Ý
&Z<ž<<<Ž
  )");
}

inline string_view standard_collation_tailoring()
{
    return string_view(
u8R"(  
&A<ä<<<Ä
&C<č<<<Č
&H<ch<<<cH<<<Ch<<<CH
&O<ô<<<Ô
&R<ř<<<Ř
&S<š<<<Š
&Z<ž<<<Ž
  )");
}


}}}}

#endif
