#include <boost/text/segmented_vector.hpp>

#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>


namespace {

    boost::text::segmented_vector<int> seg_vec;
    boost::text::segmented_vector<int> copied_seg_vec;
    std::vector<int> vec;

    std::ofstream ofs("fuzz_operations.cpp");

    bool init()
    {
        ofs << R"(#include <boost/text/segmented_vector.hpp>


int main()
{
    boost::text::segmented_vector<int> seg_vec;
    boost::text::segmented_vector<int> copied_seg_vec;

)";
        return true;
    }
    auto const dummy = init();
}

struct action_t
{
    enum op_t : uint8_t {
        insert = 0,
        push_back = 1,
        erase = 2,

        num_ops,
        first_op = insert
    };

    int value_;
    op_t op_ : 2;
};

void check()
{
    assert(seg_vec.size() == vec.size());
    assert(copied_seg_vec.size() == vec.size());

    auto vec_it = vec.begin();
    auto const vec_end = vec.end();
    auto seg_vec_it = seg_vec.begin();
    auto copied_seg_vec_it = copied_seg_vec.begin();
    for (; vec_it != vec_end; ++vec_it, ++seg_vec_it, ++copied_seg_vec_it) {
        auto const vec_x = *vec_it;
        auto const seg_vec_x = *seg_vec_it;
        auto const copied_seg_vec_x = *copied_seg_vec_it;
        assert(seg_vec_x == copied_seg_vec_x);
        assert(vec_x == copied_seg_vec_x);
        assert(vec_x == seg_vec_x);
    }
}

void push_back(int value)
{
    ofs << "    seg_vec.push_back(" << value << ");\n" << std::flush;
    ofs << "    copied_seg_vec = copied_seg_vec.push_back(" << value << ");\n" << std::flush;

    vec.push_back(value);
    seg_vec.push_back(value);
    copied_seg_vec = copied_seg_vec.push_back(value);

    check();
}

void insert(int i, int value)
{
    boost::text::segmented_vector<int> seg_vec_2 = seg_vec;
    boost::text::segmented_vector<int> copied_seg_vec_2 = copied_seg_vec;

    ofs << "    seg_vec.insert(seg_vec.begin() + " << i << ", " << value
        << ");\n"
        << std::flush;
    ofs << "    copied_seg_vec = copied_seg_vec.insert(seg_vec.begin() + " << i
        << ", " << value << ");\n"
        << std::flush;

    vec.insert(vec.begin() + i, value);
    seg_vec.insert(seg_vec.begin() + i, value);
    copied_seg_vec = copied_seg_vec.insert(copied_seg_vec.begin() + i, value);

    {
        assert(seg_vec.size() == vec.size());
        assert(copied_seg_vec.size() == vec.size());

        auto vec_it = vec.begin();
        auto const vec_end = vec.end();
        auto seg_vec_it = seg_vec.begin();
        auto copied_seg_vec_it = copied_seg_vec.begin();
        for (; vec_it != vec_end; ++vec_it, ++seg_vec_it, ++copied_seg_vec_it) {
            auto const vec_x = *vec_it;
            auto const seg_vec_x = *seg_vec_it;
            auto const copied_seg_vec_x = *copied_seg_vec_it;
            if(!(vec_x == copied_seg_vec_x)) {
                copied_seg_vec_2 =
                    copied_seg_vec_2.insert(copied_seg_vec.begin() + i, value);
            }
            if (!(vec_x == seg_vec_x)) {
                seg_vec_2.insert(seg_vec.begin() + i, value);
            }
        }
    }

    check();
}

void erase(int i)
{
    ofs << "    seg_vec.erase(seg_vec.begin() + " << i << ");\n" << std::flush;
    ofs << "    copied_seg_vec = seg_vec.erase(seg_vec.begin() + " << i
        << ");\n"
        << std::flush;

    vec.erase(vec.begin() + i);
    seg_vec.erase(seg_vec.begin() + i);
    copied_seg_vec = copied_seg_vec.erase(copied_seg_vec.begin() + i);

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
            if (action.op_ == action_t::push_back) {
                push_back(action.value_);
            } else if (action.op_ <= action_t::num_ops && !vec.empty()) {
                std::size_t const index =
                    std::size_t(std::abs(action.value_)) % vec.size();
                assert(index < vec.size());
                if (action.op_ == action_t::erase)
                    erase(index);
                else
                    insert(index, action.value_);
            }
        }
    }
    return 0;
}
