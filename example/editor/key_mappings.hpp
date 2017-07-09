#ifndef EDITOR_KEY_MAPPINGS
#define EDITOR_KEY_MAPPINGS

#include <boost/container/static_vector.hpp>
#include <boost/functional/hash.hpp>
#include <boost/text/text_view.hpp>

#include <unordered_map>


struct key_sequence_t;

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

struct key_code_t
{
    key_code_t () {}
    key_code_t (char c);
    key_code_t (key k);
    key_code_t (int mod, int key) : mod_ (mod), key_ (key) {}

    bool operator== (key_code_t rhs) const
    { return mod_ == rhs.mod_ && key_ == rhs.key_; }

    key_sequence_t operator, (key_code_t rhs) &&;

    int mod_;
    int key_;
};

struct key_sequence_t
{
    key_sequence_t () {}
    key_sequence_t (char c)
    { keys_.push_back(c); }
    key_sequence_t (key k)
    { keys_.push_back(k); }
    key_sequence_t (key_code_t k)
    { keys_.push_back(k); }

    bool operator== (key_sequence_t rhs) const
    { return keys_ == rhs.keys_; }

    key_sequence_t operator, (key_code_t k) &&
    {
        key_sequence_t retval(*this);
        keys_.push_back(k);
        return retval;
    }

    static const int max_size = 32;
    boost::container::static_vector<key_code_t, max_size> keys_;
};

inline std::size_t hash_value (key_sequence_t const & seq)
{
    boost::hash<int> hasher;
    std::size_t seed = 0;
    for (auto const key_code : seq.keys_) {
        boost::hash_combine(seed, hasher(key_code.mod_));
        boost::hash_combine(seed, hasher(key_code.key_));
    }
    return seed;
}

using key_map_t = std::unordered_map<
    key_sequence_t,
    boost::text::text_view,
    boost::hash<key_sequence_t>
>;

struct ctrl_t {};
struct alt_t {};

key_code_t operator- (ctrl_t, char c);
key_code_t operator- (ctrl_t, key k);
key_sequence_t operator- (alt_t, char c);
key_code_t operator- (alt_t, key k);

key_map_t emacs_lite ();


// implementations

inline key_sequence_t key_code_t::operator, (key_code_t rhs) &&
{
    key_sequence_t retval;
    retval.keys_.push_back(*this);
    retval.keys_.push_back(rhs);
    return retval;
}

namespace {
    ctrl_t ctrl;
    alt_t alt;
    // Silence warnings.
    bool const b_ = ([=]{ (void)ctrl; (void)alt; return true; })();
}

#endif
