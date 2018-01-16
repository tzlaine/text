#define ENABLE_DUMP 1
#include "trie_tests.hpp"

#include <boost/text/trie.hpp>

#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>


namespace {

    boost::trie::trie<boost::text::string, int> trie;
    std::map<boost::text::string, int> map;
    std::ofstream ofs("inserts.cpp");
}

struct action_t
{
    enum op_t {
        insert,
//        erase,

        num_ops
    };

    op_t op_;
    int value_;
};

void execute(action_t action, boost::text::string_view key)
{
    switch (action.op_) {
    case action_t::insert:
#if 0
        ofs << "trie.insert("
            << std::quoted(std::string(key.begin(), key.end())) << ", "
            << action.value_ << "); // key.size()=" << key.size() << "\n";
#endif
        trie.insert(key, action.value_);
        map[boost::text::string(key)] = action.value_;
        break;
#if 0
    case action_t::erase:
        trie.erase(key);
        map.erase(boost::text::string(key));
        break;
#endif
    default: break;
    }

    assert(trie.size() == map.size());

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
        if (trie_it->key != map_it->first || trie_it->value != map_it->second) {
            ofs.close();
#if ENABLE_DUMP
            dump(std::cout, trie);
#endif
        }
        assert(trie_it->key == map_it->first);
        assert(trie_it->value == map_it->second);
    }
    assert(map_it == map.end());
}

extern "C" int LLVMFuzzerTestOneInput(uint8_t const * data, size_t size)
{
    if (sizeof(action_t) <= size) {
        action_t action;
        std::memcpy(&action, data, sizeof(action_t));
        data += sizeof(action_t);
        size -= sizeof(action_t);
        if (0 <= action.op_ && action.op_ < action_t::num_ops &&
            size < INT_MAX) {
            execute(action, boost::text::string_view((char const *)data, size));
        }
    }
    return 0;
}
