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

char const case_3_3[] = R"( [caseLevel on]
    &c < k)";

char const case_3_5_a[] = "&'\\u0020'='\\u3000'";
char const case_3_5_b[] = "&'\\u0022'<<<x";

char const case_3_6_a[] = "& Z";
char const case_3_6_b[] = "& a < b";
char const case_3_6_c[] = "& a << ä";
char const case_3_6_d[] = "& a <<< A";
char const case_3_6_e[] = "& か <<<< カ";
char const case_3_6_f[] = "& v = w";

char const case_3_6_g[] = "& a < g";
char const case_3_6_h[] = "& a < h < k";
char const case_3_6_i[] = "& h << g";

char const case_3_6_j[] = "& b < q <<< Q";
char const case_3_6_k[] = "& a < x <<< X << q <<< Q < z";

char const case_3_6_l[] = "& a <* bcd-gp-s";
char const case_3_6_m[] = "& a <<* æᶏɐ";
char const case_3_6_n[] = "& p <<<* PｐＰ";
char const case_3_6_o[] = "& k <<<<* qQ";
char const case_3_6_p[] = "& v =* VwW";

char const case_3_7[] = "& k < ch";

char const case_3_9_a[] = R"(& a <<< a | '-'
& e <<< e | '-')";

char const case_3_9_b[] = "& x < abc | def / ghi";
char const case_3_9_c[] = "& x < def / ghi";
char const case_3_9_d[] = "& x < abc | def";

char const case_3_10_a[] = "&[before 2] a << à";
char const case_3_10_b[] = "&[before 2] a < à # error";
char const case_3_10_c[] = R"(&[before 2] a <<< à # error
)";

char const case_3_11[] = "& [first tertiary ignorable] << à";

char const case_3_12_a[] = "[import de-u-co-phonebk]";
char const case_3_12_b[] = "[import und-u-co-search]";
char const case_3_12_c[] = "[import ja-u-co-private-kana]";

char const case_3_12_d[] = "[suppressContractions [Љ-ґ]]";
char const case_3_12_e[] = "[optimize [Ά-ώ]]";

char const case_3_12_f[] = R"([caseLevel on]
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

char const case_3_13[] = "[reorder Grek Latn digit]";
