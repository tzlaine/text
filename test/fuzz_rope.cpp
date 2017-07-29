#include <boost/text/rope.hpp>


namespace {

    int const num_ropes = 4;
    boost::text::rope ropes[num_ropes];

}

struct action_t
{
    enum op_t {
        insert_rv,
        erase_rv,
        replace_rv,

        num_ops
    };

    op_t op_;
    int lhs_;
    int rhs_;
    int first_;
    int last_;
    int old_first_; // Used as insertion point, and in replace.
    int old_last_;  // Used in replace.
};

void execute (action_t action, uint8_t const * data, size_t size)
{
    switch (action.op_) {
    case action_t::insert_rv:
        if (0 <= action.old_first_ &&
            action.old_first_ < ropes[action.lhs_].size()
            &&
            0 <= action.first_ &&
            action.first_ <= action.last_ &&
            action.last_ < ropes[action.rhs_].size()) {
            ropes[action.lhs_].insert(action.old_first_, ropes[action.rhs_](action.first_, action.last_));
        }
        break;
    case action_t::erase_rv:
        if (0 <= action.first_ &&
            action.first_ <= action.last_ &&
            action.last_ < ropes[action.lhs_].size()) {
            ropes[action.lhs_].erase(ropes[action.lhs_](action.first_, action.last_));
        }
        break;
    case action_t::replace_rv:
        if (0 <= action.old_first_ &&
            action.old_first_ <= action.old_last_ &&
            action.old_last_ < ropes[action.lhs_].size()
            &&
            0 <= action.first_ &&
            action.first_ <= action.last_ &&
            action.last_ < ropes[action.rhs_].size()) {
            ropes[action.lhs_].replace(
                ropes[action.lhs_](action.old_first_, action.old_last_),
                ropes[action.rhs_](action.first_, action.last_)
            );
        }
        break;
    default: break;
    }
}

extern "C"
int LLVMFuzzerTestOneInput (uint8_t const * data, size_t size)
{
    if (sizeof(action_t) <= size) {
        action_t action;
        std::memcpy(&action, data, sizeof(action_t));
        data += sizeof(action_t);
        size -= sizeof(action_t);
        if (0 <= action.op_ && action.op_ < action_t::num_ops &&
            0 <= action.lhs_ && action.lhs_ < num_ropes &&
            0 <= action.rhs_ && action.rhs_ < num_ropes) {
            execute(action, data, size);
        }
    }
    return 0;
}
