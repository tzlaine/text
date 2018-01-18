#if ENABLE_DUMP
#    include <boost/text/string.hpp>
#    include <iostream>
#    define private public
#    include <boost/text/trie.hpp>
#    undef private

namespace std {
    ostream & operator<<(ostream & os, vector<int> const & vec)
    {
        os << "[";
        for (auto x : vec) {
            os << " " << x;
        }
        os << " ]";
        return os;
    }
}

template<typename Key, typename Value>
void dump(
    std::ostream & os,
    boost::trie::detail::trie_node_t<Key, Value> const & node,
    int indent = 1)
{
    auto ind = [indent] {
        return boost::text::repeated_string_view("  ", indent);
    };
    os << ind() << "==== NODE " << (void *)(&node) << " ====\n"
       << ind() << "  parent=" << node.parent_;
    if (node.parent_ == &node)
        os << " (self)";
    os << "\n";
    os << ind() << "  index_within_parent=" << node.index_within_parent_
       << "\n";
    if (node.value_) {
        os << ind() << "  value=\"" << node.value_->key << "\", "
           << node.value_->value << "\n";
    } else {
        os << ind() << "  value=[empty]\n";
    }
    os << ind() << "  " << node.keys_.size() << " keys:";
    for (auto k : node.keys_) {
        os << " " << k;
    }
    os << "\n" << ind() << "  " << node.children_.size() << " children:\n";
    for (auto const & child : node.children_) {
        dump(os, *child, indent + 1);
    }
}
template<typename Key, typename Value>
void dump(std::ostream & os, boost::trie::trie<Key, Value> const & trie)
{
    os << "==== TRIE ====\n"
       << "  " << trie.size() << " elements\n"
       << "  nodes:\n";
    dump(os, trie.root_);
}
#endif
