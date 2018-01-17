#define ENABLE_DUMP 1
#include "trie_tests.hpp"

#include <boost/text/trie.hpp>

#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>


namespace {

    boost::trie::trie<boost::text::string, int> trie;
    std::map<boost::text::string, int> map;

    std::ofstream ofs("fuzz_operations.cpp");
}

struct action_t
{
    enum op_t : uint8_t {
        insert = 0,
        erase = 1,

        num_ops,
        first_op = insert
    };

    int value_;
    op_t op_ : 1;
};

void check()
{
    assert(trie.size() == map.size());

    {
        auto trie_it = trie.begin();
        auto const trie_end = trie.end();
        auto map_it = map.begin();
        for (; trie_it != trie_end; ++trie_it, ++map_it) {
            if (trie_it->key != map_it->first) {
                std::cout << trie_it->key.size() << "\n"
                          << trie_it->key << "\n!=\n"
                          << map_it->first.size() << "\n"
                          << map_it->first << "\n";
            }
            if (trie_it->value != map_it->second) {
                std::cout << trie_it->value << " != " << map_it->second << "\n";
            }
            if (trie_it->key != map_it->first ||
                trie_it->value != map_it->second) {
#if ENABLE_DUMP
                dump(std::cout, trie);
#endif
            }
            assert(trie_it->key == map_it->first);
            assert(trie_it->value == map_it->second);
        }
        assert(map_it == map.end());
    }

    {
        auto trie_it = trie.rbegin();
        auto const trie_end = trie.rend();
        auto map_it = map.rbegin();
        for (; trie_it != trie_end; ++trie_it, ++map_it) {
            if (trie_it->key != map_it->first) {
                std::cout << trie_it->key.size() << "\n"
                          << trie_it->key << "\n!=\n"
                          << map_it->first.size() << "\n"
                          << map_it->first << "\n";
            }
            if (trie_it->value != map_it->second) {
                std::cout << trie_it->value << " != " << map_it->second << "\n";
            }
            if (trie_it->key != map_it->first ||
                trie_it->value != map_it->second) {
#if ENABLE_DUMP
                dump(std::cout, trie);
#endif
            }
            assert(trie_it->key == map_it->first);
            assert(trie_it->value == map_it->second);
        }
        assert(map_it == map.rend());
    }
}

void insert(boost::text::string_view key, int value)
{
    ofs << "trie.insert(" << std::quoted(std::string(key.begin(), key.end()))
        << ", " << value << "); // key.size()=" << key.size() << "\n"
        << std::flush;
    trie.insert(key, value);
    map.insert(
        std::pair<boost::text::string, int>(boost::text::string(key), value));

    check();
}

void erase(int which)
{
    auto const it = std::next(trie.begin(), which);
    auto const & key = it->key;
    ofs << "trie.erase(" << std::quoted(std::string(key.begin(), key.end()))
        << "); // key.size()=" << key.size() << "\n"
        << std::flush;
    trie.erase(key);
    map.erase(boost::text::string(key));

    check();
}

extern "C" int LLVMFuzzerTestOneInput(uint8_t const * data, size_t size)
{
    if (sizeof(action_t) <= size) {
        action_t action;
        std::memcpy(&action, data, sizeof(action_t));
        data += sizeof(action_t);
        size -= sizeof(action_t);
        if (action_t::first_op <= action.op_ &&
            action.op_ < action_t::num_ops && size < INT_MAX) {
            if (action.op_ == action_t::erase) {
                if (!trie.empty()) {
                    std::size_t const index =
                        std::size_t(std::abs(action.value_)) % trie.size();
                    assert(index < trie.size());
                    erase(index);
                }
            } else {
                auto str = boost::text::string_view((char const *)data, size);
                if (std::all_of(str.begin(), str.end(), [](char c) {
                        return std::isprint(c);
                    })) {
                    insert(str, action.value_);
                }
            }
        }
    }
    return 0;
}
