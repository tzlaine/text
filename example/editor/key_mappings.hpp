#ifndef EDITOR_KEY_MAPPINGS
#define EDITOR_KEY_MAPPINGS

#include <boost/container/static_vector.hpp>
#include <boost/functional/hash.hpp>
#include <boost/text/text_view.hpp>

#include <unordered_map>


struct key_code_t
{
    int f;
    char32_t b;
};

enum key {
    up,
    down,
    left,
    right,
    home,
    end,
    backspace,
    delete_,
    page_up,
    page_down
};

struct key_sequence_t
{
    key_sequence_t () {}
    key_sequence_t (key k)
    {
        // TODO
    }

    static const int max_size = 32;
    boost::container::static_vector<key_code_t, max_size> keys_;
};

inline std::size_t hash_value (key_sequence_t const & seq)
{
    boost::hash<int> hasher;
    std::size_t seed = 0;
    for (auto const key_code : seq.keys_) {
        boost::hash_combine(seed, hasher(key_code.f));
        boost::hash_combine(seed, hasher(key_code.b));
    }
    return seed;
}

using key_map_t = std::unordered_map<
    key_sequence_t,
    boost::text::text_view,
    boost::hash<key_sequence_t>
>;

struct ctrl {};
struct alt {};

key_map_t emacs_lite ();

#endif
