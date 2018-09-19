#include <iostream>


template<typename Seq>
struct seq_dumper
{
    seq_dumper(Seq const & seq) : first_(std::begin(seq)), last_(std::end(seq))
    {}

    friend std::ostream & operator<<(std::ostream & os, seq_dumper sd)
    {
        for (auto it = sd.first_; it != sd.last_; ++it) {
            os << *it << " ";
        }
        return os;
    }

    typename Seq::const_iterator first_;
    typename Seq::const_iterator last_;
};

template<typename Seq>
seq_dumper<Seq> dump(Seq const & seq)
{
    return seq_dumper<Seq>(seq);
}

char const case_3_3[] = u8R"( [caseLevel on]
    &c < k)";

char const case_3_5_a[] = u8"&'\\u0020'='\\u3000'";
char const case_3_5_b[] = u8"&'\\u0022'<<<x";

char const case_3_6_a[] = u8"& Z";
char const case_3_6_b[] = u8"& a < b";
char const case_3_6_c[] = u8"& a << ä";
char const case_3_6_d[] = u8"& a <<< A";
char const case_3_6_e[] = u8"& か <<<< カ";
char const case_3_6_f[] = u8"& v = w";

char const case_3_6_g[] = u8"& a < g";
char const case_3_6_h[] = u8"& a < h < k";
char const case_3_6_i[] = u8"& h << g";

char const case_3_6_j[] = u8"& b < q <<< Q";
char const case_3_6_k[] = u8"& a < x <<< X << q <<< Q < z";

char const case_3_6_l[] = u8"& a <* bcd-gp-s";
char const case_3_6_m[] = u8"& a <<* æᶏɐ";
char const case_3_6_n[] = u8"& p <<<* PｐＰ";
char const case_3_6_o[] = u8"& k <<<<* qQ";
char const case_3_6_p[] = u8"& v =* VwW";

char const case_3_7[] = u8"& k < ch";

char const case_3_9_a[] = u8R"(& a <<< a | '-'
& e <<< e | '-')";

char const case_3_9_b[] = u8"& x < abc | def / ghi";
char const case_3_9_c[] = u8"& x < def / ghi";
char const case_3_9_d[] = u8"& x < abc | def";

char const case_3_10_a[] = u8"&[before 2] a << à";
char const case_3_10_b[] = u8"&[before 2] a < à # error";
char const case_3_10_c[] = R"(&[before 2] a <<< à # error
)";

char const case_3_11[] = u8"& [first tertiary ignorable] << à";

char const case_3_12_a[] = u8"[import de-u-co-phonebk]";
char const case_3_12_b[] = u8"[import und-u-co-search]";
char const case_3_12_c[] = u8"[import ja-u-co-private-kana]";

char const case_3_12_d[] = u8"[suppressContractions [Љ-ґ]]";
char const case_3_12_e[] = u8"[optimize [Ά-ώ]]";

char const case_3_12_f[] = u8R"([caseLevel on]
    &Z
    < æ <<< Æ
    < å <<< Å <<< aa <<< aA <<< Aa <<< AA
    < ä <<< Ä
    < ö <<< Ö << ű <<< Ű
    < ő <<< Ő << ø <<< Ø
    &V <<<* wW
    &Y <<<* üÜ
    &[last non-ignorable]
    # The following is equivalent to <亜<唖<娃...
    <* 亜唖娃阿哀愛挨姶逢葵茜穐悪握渥旭葦芦
    <* 鯵梓圧斡扱
)";

char const case_3_13[] = u8"[reorder Grek Latn digit]";
