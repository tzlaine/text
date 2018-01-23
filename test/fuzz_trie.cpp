#define ENABLE_DUMP 0
#include "trie_tests.hpp"

#include <boost/text/trie.hpp>
#include <boost/text/trie_map.hpp>
#include <boost/text/trie_set.hpp>
#include <boost/text/string.hpp>

#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>


namespace {

    boost::trie::trie<boost::text::string, int> trie;
    boost::trie::trie_map<boost::text::string, int> trie_map;
    boost::trie::trie_set<boost::text::string> trie_set;
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
    assert(trie_map.size() == map.size());
    assert(trie_set.size() == trie_set.size());

    {
        auto trie_map_it = trie_map.begin();
        auto const trie_map_end = trie_map.end();
        auto trie_set_it = trie_set.begin();
        auto map_it = map.begin();
        for (; trie_map_it != trie_map_end;
             ++trie_map_it, ++trie_set_it, ++map_it) {
            if (trie_map_it->key != map_it->first) {
                std::cout << trie_map_it->key.size() << "\n"
                          << trie_map_it->key << "\n!=\n"
                          << map_it->first.size() << "\n"
                          << map_it->first << "\n";
            }
            if (trie_map_it->value != map_it->second) {
                std::cout << trie_map_it->value << " != " << map_it->second
                          << "\n";
            }
            if (trie_map_it->key != map_it->first ||
                trie_map_it->value != map_it->second) {
#if ENABLE_DUMP
                dump(std::cout, trie_map);
#endif
            }
            assert(trie_map_it->key == map_it->first);
            assert(trie_map_it->value == map_it->second);

            assert(*trie_set_it == map_it->first);

            assert(trie.contains(trie_map_it->key));
            assert(trie[trie_map_it->key] == trie_map_it->value);
        }
        assert(map_it == map.end());
    }

    {
        auto trie_map_it = trie_map.rbegin();
        auto const trie_map_end = trie_map.rend();
        auto trie_set_it = trie_set.rbegin();
        auto map_it = map.rbegin();
        for (; trie_map_it != trie_map_end;
             ++trie_map_it, ++trie_set_it, ++map_it) {
            if (trie_map_it->key != map_it->first) {
                std::cout << trie_map_it->key.size() << "\n"
                          << trie_map_it->key << "\n!=\n"
                          << map_it->first.size() << "\n"
                          << map_it->first << "\n";
            }
            if (trie_map_it->value != map_it->second) {
                std::cout << trie_map_it->value << " != " << map_it->second
                          << "\n";
            }
            if (trie_map_it->key != map_it->first ||
                trie_map_it->value != map_it->second) {
#if ENABLE_DUMP
                dump(std::cout, trie_map);
#endif
            }
            assert(trie_map_it->key == map_it->first);
            assert(trie_map_it->value == map_it->second);

            assert(*trie_set_it == map_it->first);
        }
        assert(map_it == map.rend());
    }
}

void insert(boost::text::string_view key, int value)
{
#if 201402L <= __cplusplus
    ofs << "trie.insert(" << std::quoted(std::string(key.begin(), key.end()))
        << ", " << value << "); // key.size()=" << key.size() << "\n"
#    if 0
        << "trie_map.insert(" << std::quoted(std::string(key.begin(), key.end()))
        << ", " << value << "); // key.size()=" << key.size() << "\n"
        << "map.insert(std::make_pair(boost::text::string("
        << std::quoted(std::string(key.begin(), key.end())) << "), " << value
        << "));\n"
#    endif
        << std::flush;
#endif
    trie.insert(key, value);
    trie_map.insert(key, value);
    trie_set.insert(key);
    map.insert(std::make_pair(boost::text::string(key), value));

    check();
}

void erase(int which)
{
    auto const it = std::next(trie_map.begin(), which);
    auto const key = it->key;
#if 201402L <= __cplusplus
    ofs << "map.erase(" << std::quoted(std::string(key.begin(), key.end()))
        << "); // key.size()=" << key.size() << "\n"
#    if 0
        << "trie_map.erase(" << std::quoted(std::string(key.begin(), key.end()))
        << "); // key.size()=" << key.size() << "\n"
        << "map.erase(" << std::quoted(std::string(key.begin(), key.end()))
        << ");\n"
#    endif
        << std::flush;
#endif
    trie.erase(key);
    trie_map.erase(key);
    trie_set.erase(key);
    map.erase(key);

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
                if (!trie_map.empty()) {
                    std::size_t const index =
                        std::size_t(std::abs(action.value_)) % trie_map.size();
                    assert(index < trie_map.size());
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
