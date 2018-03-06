#include <boost/text/bidirectional.hpp>

#include <gtest/gtest.h>


using namespace boost::text;
using namespace boost::text::detail;


// https://unicode.org/reports/tr9/#BD13
TEST(detail_bidi, compute_run_sequences)
{
    auto run_used = [](level_run r) { return r.used_; };

    // Using bidi_prop_t::L for all portions of the examples called "text".

    {
        props_and_embeddings_t paes = {
            // text1
            {bidi_prop_t::L, false, 0},
            {bidi_prop_t::L, false, 0},
            {bidi_prop_t::L, false, 0},

            {bidi_prop_t::RLE, false, 0},

            // text2
            {bidi_prop_t::L, false, 1},
            {bidi_prop_t::L, false, 1},
            {bidi_prop_t::L, false, 1},

            {bidi_prop_t::PDF, false, 1},

            {bidi_prop_t::RLE, false, 1},

            // text3
            {bidi_prop_t::L, false, 1},
            {bidi_prop_t::L, false, 1},
            {bidi_prop_t::L, false, 1},

            {bidi_prop_t::PDF, false, 1},

            // text4
            {bidi_prop_t::L, false, 0},
            {bidi_prop_t::L, false, 0},
            {bidi_prop_t::L, false, 0},
        };

        auto const runs = find_all_runs(paes);

        EXPECT_EQ(runs.size(), 3u);
        EXPECT_TRUE(std::none_of(runs.begin(), runs.end(), run_used));
        EXPECT_EQ(runs[0].first_ - paes.begin(), 0);
        EXPECT_EQ(runs[0].last_ - paes.begin(), 4);
        EXPECT_EQ(runs[1].first_ - paes.begin(), 4);
        EXPECT_EQ(runs[1].last_ - paes.begin(), 13);
        EXPECT_EQ(runs[2].first_ - paes.begin(), 13);
        EXPECT_EQ(runs[2].last_ - paes.begin(), 16);

        auto const run_sequences = find_run_sequences(paes);

        EXPECT_EQ(run_sequences.size(), 3u);
        EXPECT_EQ(run_sequences[0].runs_[0].first_ - paes.begin(), 0);
        EXPECT_EQ(run_sequences[0].runs_[0].last_ - paes.begin(), 4);
        EXPECT_EQ(run_sequences[1].runs_[0].first_ - paes.begin(), 4);
        EXPECT_EQ(run_sequences[1].runs_[0].last_ - paes.begin(), 13);
        EXPECT_EQ(run_sequences[2].runs_[0].first_ - paes.begin(), 13);
        EXPECT_EQ(run_sequences[2].runs_[0].last_ - paes.begin(), 16);
    }

    {
        props_and_embeddings_t paes = {
            // text1
            {bidi_prop_t::L, false, 0},
            {bidi_prop_t::L, false, 0},
            {bidi_prop_t::L, false, 0},

            {bidi_prop_t::RLI, false, 0},

            // text2
            {bidi_prop_t::L, false, 1},
            {bidi_prop_t::L, false, 1},
            {bidi_prop_t::L, false, 1},

            {bidi_prop_t::PDI, false, 0},

            {bidi_prop_t::RLI, false, 0},

            // text3
            {bidi_prop_t::L, false, 1},
            {bidi_prop_t::L, false, 1},
            {bidi_prop_t::L, false, 1},

            {bidi_prop_t::PDI, false, 0},

            // text4
            {bidi_prop_t::L, false, 0},
            {bidi_prop_t::L, false, 0},
            {bidi_prop_t::L, false, 0},
        };

        auto const runs = find_all_runs(paes);

        EXPECT_EQ(runs.size(), 5u);
        EXPECT_TRUE(std::none_of(runs.begin(), runs.end(), run_used));
        EXPECT_EQ(runs[0].first_ - paes.begin(), 0);
        EXPECT_EQ(runs[0].last_ - paes.begin(), 4);
        EXPECT_EQ(runs[1].first_ - paes.begin(), 4);
        EXPECT_EQ(runs[1].last_ - paes.begin(), 7);
        EXPECT_EQ(runs[2].first_ - paes.begin(), 7);
        EXPECT_EQ(runs[2].last_ - paes.begin(), 9);
        EXPECT_EQ(runs[3].first_ - paes.begin(), 9);
        EXPECT_EQ(runs[3].last_ - paes.begin(), 12);
        EXPECT_EQ(runs[4].first_ - paes.begin(), 12);
        EXPECT_EQ(runs[4].last_ - paes.begin(), 16);

        auto const run_sequences = find_run_sequences(paes);

        EXPECT_EQ(run_sequences.size(), 3u);
        EXPECT_EQ(run_sequences[0].runs_[0].first_ - paes.begin(), 0);
        EXPECT_EQ(run_sequences[0].runs_[0].last_ - paes.begin(), 4);
        EXPECT_EQ(run_sequences[0].runs_[1].first_ - paes.begin(), 7);
        EXPECT_EQ(run_sequences[0].runs_[1].last_ - paes.begin(), 9);
        EXPECT_EQ(run_sequences[0].runs_[2].first_ - paes.begin(), 12);
        EXPECT_EQ(run_sequences[0].runs_[2].last_ - paes.begin(), 16);
        EXPECT_EQ(run_sequences[1].runs_[0].first_ - paes.begin(), 4);
        EXPECT_EQ(run_sequences[1].runs_[0].last_ - paes.begin(), 7);
        EXPECT_EQ(run_sequences[2].runs_[0].first_ - paes.begin(), 9);
        EXPECT_EQ(run_sequences[2].runs_[0].last_ - paes.begin(), 12);
    }

    {
        props_and_embeddings_t paes = {
            // text1
            {bidi_prop_t::L, false, 0},
            {bidi_prop_t::L, false, 0},
            {bidi_prop_t::L, false, 0},

            {bidi_prop_t::RLI, false, 0},

            // text2
            {bidi_prop_t::L, false, 1},
            {bidi_prop_t::L, false, 1},
            {bidi_prop_t::L, false, 1},

            {bidi_prop_t::LRI, false, 1},

            // text3
            {bidi_prop_t::L, false, 2},
            {bidi_prop_t::L, false, 2},
            {bidi_prop_t::L, false, 2},

            {bidi_prop_t::RLE, false, 2},

            // text4
            {bidi_prop_t::L, false, 3},
            {bidi_prop_t::L, false, 3},
            {bidi_prop_t::L, false, 3},

            {bidi_prop_t::PDF, false, 3},

            // text5
            {bidi_prop_t::L, false, 2},
            {bidi_prop_t::L, false, 2},
            {bidi_prop_t::L, false, 2},

            {bidi_prop_t::PDI, false, 1},

            // text6
            {bidi_prop_t::L, false, 1},
            {bidi_prop_t::L, false, 1},
            {bidi_prop_t::L, false, 1},

            {bidi_prop_t::PDI, false, 0},

            // text7
            {bidi_prop_t::L, false, 0},
            {bidi_prop_t::L, false, 0},
            {bidi_prop_t::L, false, 0},
        };

        auto const runs = find_all_runs(paes);

        EXPECT_EQ(runs.size(), 7u);
        EXPECT_TRUE(std::none_of(runs.begin(), runs.end(), run_used));
        EXPECT_EQ(runs[0].first_ - paes.begin(), 0);
        EXPECT_EQ(runs[0].last_ - paes.begin(), 4);
        EXPECT_EQ(runs[1].first_ - paes.begin(), 4);
        EXPECT_EQ(runs[1].last_ - paes.begin(), 8);
        EXPECT_EQ(runs[2].first_ - paes.begin(), 8);
        EXPECT_EQ(runs[2].last_ - paes.begin(), 12);
        EXPECT_EQ(runs[3].first_ - paes.begin(), 12);
        EXPECT_EQ(runs[3].last_ - paes.begin(), 16);
        EXPECT_EQ(runs[4].first_ - paes.begin(), 16);
        EXPECT_EQ(runs[4].last_ - paes.begin(), 19);
        EXPECT_EQ(runs[5].first_ - paes.begin(), 19);
        EXPECT_EQ(runs[5].last_ - paes.begin(), 23);
        EXPECT_EQ(runs[6].first_ - paes.begin(), 23);
        EXPECT_EQ(runs[6].last_ - paes.begin(), 27);

        auto const run_sequences = find_run_sequences(paes);

        EXPECT_EQ(run_sequences.size(), 5u);
        EXPECT_EQ(run_sequences[0].runs_[0].first_ - paes.begin(), 0);
        EXPECT_EQ(run_sequences[0].runs_[0].last_ - paes.begin(), 4);
        EXPECT_EQ(run_sequences[0].runs_[1].first_ - paes.begin(), 23);
        EXPECT_EQ(run_sequences[0].runs_[1].last_ - paes.begin(), 27);
        EXPECT_EQ(run_sequences[1].runs_[0].first_ - paes.begin(), 4);
        EXPECT_EQ(run_sequences[1].runs_[0].last_ - paes.begin(), 8);
        EXPECT_EQ(run_sequences[1].runs_[1].first_ - paes.begin(), 19);
        EXPECT_EQ(run_sequences[1].runs_[1].last_ - paes.begin(), 23);
        EXPECT_EQ(run_sequences[2].runs_[0].first_ - paes.begin(), 8);
        EXPECT_EQ(run_sequences[2].runs_[0].last_ - paes.begin(), 12);
        EXPECT_EQ(run_sequences[3].runs_[0].first_ - paes.begin(), 12);
        EXPECT_EQ(run_sequences[3].runs_[0].last_ - paes.begin(), 16);
        EXPECT_EQ(run_sequences[4].runs_[0].first_ - paes.begin(), 16);
        EXPECT_EQ(run_sequences[4].runs_[0].last_ - paes.begin(), 19);
    }
}
