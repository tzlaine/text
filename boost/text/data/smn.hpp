// Warning! This file is autogenerated.
#ifndef BOOST_TEXT_DATA_SMN_HPP
#define BOOST_TEXT_DATA_SMN_HPP

#include <boost/text/string_view.hpp>


namespace boost { namespace text { namespace data { namespace smn {

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
  
  
&A<â<<<Â<<à<<<À
&C<č<<<Č
&D<đ<<<Đ
&Ŋ<<ñ<<<Ñ<<ń<<<Ń
&S<š<<<Š
&Z<ž<<<Ž<æ<<<Æ<ø<<<Ø<å<<<Å<ã<<<Ã<ä<<<Ä<á<<<Á<ö<<<Ö
  
  )");
}

inline string_view standard_collation_tailoring()
{
    return string_view(
u8R"(  
&A<â<<<Â<<à<<<À
&C<č<<<Č
&D<đ<<<Đ
&Ŋ<<ñ<<<Ñ<<ń<<<Ń
&S<š<<<Š
&Z<ž<<<Ž<æ<<<Æ<ø<<<Ø<å<<<Å<ã<<<Ã<ä<<<Ä<á<<<Á<ö<<<Ö
  )");
}


}}}}

#endif
