#ifndef EDITOR_KEY_MAPPINGS
#define EDITOR_KEY_MAPPINGS

#include <boost/container/static_vector.hpp>
#include <boost/function.hpp>
#include <boost/functional/hash.hpp>
#include <boost/optional.hpp>
#include <boost/text/text_view.hpp>

#include <unordered_map>


struct app_state_t;
struct key_sequence_t;
struct screen_pos_t;

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
    static const int max_size = 32;

    using iterator =
        boost::container::static_vector<key_code_t, max_size>::const_iterator;

    key_sequence_t () {}
    key_sequence_t (char c)
    { keys_.push_back(c); }
    key_sequence_t (key k)
    { keys_.push_back(k); }
    key_sequence_t (key_code_t k)
    { keys_.push_back(k); }

    bool operator== (key_sequence_t rhs) const
    { return keys_ == rhs.keys_; }

    iterator begin () const
    { return keys_.begin(); }

    iterator end () const
    { return keys_.end(); }

    void append (key_code_t k)
    { keys_.push_back(k); }

    key_sequence_t operator, (key_code_t k) &&
    {
        key_sequence_t retval(*this);
        keys_.push_back(k);
        return retval;
    }

private:
    boost::container::static_vector<key_code_t, max_size> keys_;
};

inline std::size_t hash_value (key_sequence_t const & seq)
{
    boost::hash<int> hasher;
    std::size_t seed = 0;
    for (auto const key_code : seq) {
        boost::hash_combine(seed, hasher(key_code.mod_));
        boost::hash_combine(seed, hasher(key_code.key_));
    }
    return seed;
}

using command_t = boost::function<
    boost::optional<app_state_t> (app_state_t, screen_pos_t)
>;

using key_map_t = std::unordered_map<
    key_sequence_t,
    command_t,
    boost::hash<key_sequence_t>
>;

struct ctrl_t {};
struct alt_t {};

extern ctrl_t ctrl;
extern alt_t alt;

key_code_t operator- (ctrl_t, char c);
key_code_t operator- (ctrl_t, key k);
key_sequence_t operator- (alt_t, char c);
key_code_t operator- (alt_t, key k);

key_map_t emacs_lite ();


// implementations

inline key_sequence_t key_code_t::operator, (key_code_t rhs) &&
{
    key_sequence_t retval;
    retval.append(*this);
    retval.append(rhs);
    return retval;
}

#endif
