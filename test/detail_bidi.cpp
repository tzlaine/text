#include <boost/text/bidirectional.hpp>

#include <gtest/gtest.h>


using namespace boost::text;
using namespace boost::text::detail;


// https://unicode.org/reports/tr9/#BD13
TEST(detail_bidi, find_run_sequences_)
{
    auto run_used = [](level_run r) { return r.used_; };

    // Using bidi_prop_t::L for all portions of the examples called "text".

    {
        props_and_embeddings_t paes = {
            // text1
            {0, 0, bidi_prop_t::L, false},
            {0, 0, bidi_prop_t::L, false},
            {0, 0, bidi_prop_t::L, false},

            {0, 0, bidi_prop_t::RLE, false},

            // text2
            {0, 1, bidi_prop_t::L, false},
            {0, 1, bidi_prop_t::L, false},
            {0, 1, bidi_prop_t::L, false},

            {0, 1, bidi_prop_t::PDF, false},

            {0, 1, bidi_prop_t::RLE, false},

            // text3
            {0, 1, bidi_prop_t::L, false},
            {0, 1, bidi_prop_t::L, false},
            {0, 1, bidi_prop_t::L, false},

            {0, 1, bidi_prop_t::PDF, false},

            // text4
            {0, 0, bidi_prop_t::L, false},
            {0, 0, bidi_prop_t::L, false},
            {0, 0, bidi_prop_t::L, false},
        };

        auto runs = find_all_runs(paes.begin(), paes.end());

        EXPECT_EQ(runs.size(), 3u);
        EXPECT_TRUE(std::none_of(runs.begin(), runs.end(), run_used));
        EXPECT_EQ(runs[0].first_ - paes.begin(), 0);
        EXPECT_EQ(runs[0].last_ - paes.begin(), 4);
        EXPECT_EQ(runs[1].first_ - paes.begin(), 4);
        EXPECT_EQ(runs[1].last_ - paes.begin(), 13);
        EXPECT_EQ(runs[2].first_ - paes.begin(), 13);
        EXPECT_EQ(runs[2].last_ - paes.begin(), 16);

        auto const run_sequences = find_run_sequences(paes, runs);

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
            {0, 0, bidi_prop_t::L, false},
            {0, 0, bidi_prop_t::L, false},
            {0, 0, bidi_prop_t::L, false},

            {0, 0, bidi_prop_t::RLI, false},

            // text2
            {0, 1, bidi_prop_t::L, false},
            {0, 1, bidi_prop_t::L, false},
            {0, 1, bidi_prop_t::L, false},

            {0, 0, bidi_prop_t::PDI, false},

            {0, 0, bidi_prop_t::RLI, false},

            // text3
            {0, 1, bidi_prop_t::L, false},
            {0, 1, bidi_prop_t::L, false},
            {0, 1, bidi_prop_t::L, false},

            {0, 0, bidi_prop_t::PDI, false},

            // text4
            {0, 0, bidi_prop_t::L, false},
            {0, 0, bidi_prop_t::L, false},
            {0, 0, bidi_prop_t::L, false},
        };

        auto runs = find_all_runs(paes.begin(), paes.end());

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

        auto const run_sequences = find_run_sequences(paes, runs);

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
            {0, 0, bidi_prop_t::L, false},
            {0, 0, bidi_prop_t::L, false},
            {0, 0, bidi_prop_t::L, false},

            {0, 0, bidi_prop_t::RLI, false},

            // text2
            {0, 1, bidi_prop_t::L, false},
            {0, 1, bidi_prop_t::L, false},
            {0, 1, bidi_prop_t::L, false},

            {0, 1, bidi_prop_t::LRI, false},

            // text3
            {0, 2, bidi_prop_t::L, false},
            {0, 2, bidi_prop_t::L, false},
            {0, 2, bidi_prop_t::L, false},

            {0, 2, bidi_prop_t::RLE, false},

            // text4
            {0, 3, bidi_prop_t::L, false},
            {0, 3, bidi_prop_t::L, false},
            {0, 3, bidi_prop_t::L, false},

            {0, 3, bidi_prop_t::PDF, false},

            // text5
            {0, 2, bidi_prop_t::L, false},
            {0, 2, bidi_prop_t::L, false},
            {0, 2, bidi_prop_t::L, false},

            {0, 1, bidi_prop_t::PDI, false},

            // text6
            {0, 1, bidi_prop_t::L, false},
            {0, 1, bidi_prop_t::L, false},
            {0, 1, bidi_prop_t::L, false},

            {0, 0, bidi_prop_t::PDI, false},

            // text7
            {0, 0, bidi_prop_t::L, false},
            {0, 0, bidi_prop_t::L, false},
            {0, 0, bidi_prop_t::L, false},
        };

        auto runs = find_all_runs(paes.begin(), paes.end());

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

        auto const run_sequences = find_run_sequences(paes, runs);

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

TEST(detail_bidi, steps_W1_through_W7)
{
    // W1
    {
        props_and_embeddings_t paes = {
            {0, 0, bidi_prop_t::AL, false},
            {0, 0, bidi_prop_t::NSM, false},
            {0, 0, bidi_prop_t::NSM, false},
        };

        auto runs = find_all_runs(paes.begin(), paes.end());
        auto run_sequences = find_run_sequences(paes, runs);
        EXPECT_EQ(run_sequences.size(), 1u);

        find_sos_eos(run_sequences, 1); // 1 implies sos==R
        w1(run_sequences[0]);

        auto it = run_sequences[0].begin();
        EXPECT_EQ(
            std::distance(run_sequences[0].begin(), run_sequences[0].end()), 3);
        EXPECT_EQ(it->prop_, bidi_prop_t::AL);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::AL);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::AL);
    }
    {
        props_and_embeddings_t paes = {
            {0, 0, bidi_prop_t::NSM, false},
        };

        auto runs = find_all_runs(paes.begin(), paes.end());
        auto run_sequences = find_run_sequences(paes, runs);
        EXPECT_EQ(run_sequences.size(), 1u);

        find_sos_eos(run_sequences, 1); // 1 implies sos==R
        w1(run_sequences[0]);

        auto it = run_sequences[0].begin();
        EXPECT_EQ(
            std::distance(run_sequences[0].begin(), run_sequences[0].end()), 1);
        EXPECT_EQ(it->prop_, bidi_prop_t::R);
    }
    {
        props_and_embeddings_t paes = {
            {0, 0, bidi_prop_t::LRI, false},
            {0, 0, bidi_prop_t::NSM, false},
        };

        auto runs = find_all_runs(paes.begin(), paes.end());
        auto run_sequences = find_run_sequences(paes, runs);
        EXPECT_EQ(run_sequences.size(), 1u);

        find_sos_eos(run_sequences, 1); // 1 implies sos==R
        w1(run_sequences[0]);

        auto it = run_sequences[0].begin();
        EXPECT_EQ(
            std::distance(run_sequences[0].begin(), run_sequences[0].end()), 2);
        EXPECT_EQ(it->prop_, bidi_prop_t::LRI);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::ON);
    }
    {
        props_and_embeddings_t paes = {
            {0, 0, bidi_prop_t::PDI, true},
            {0, 0, bidi_prop_t::NSM, false},
        };

        auto runs = find_all_runs(paes.begin(), paes.end());
        auto run_sequences = find_run_sequences(paes, runs);
        EXPECT_EQ(run_sequences.size(), 1u);

        find_sos_eos(run_sequences, 1); // 1 implies sos==R
        w1(run_sequences[0]);

        auto it = run_sequences[0].begin();
        EXPECT_EQ(
            std::distance(run_sequences[0].begin(), run_sequences[0].end()), 2);
        EXPECT_EQ(it->prop_, bidi_prop_t::PDI);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::ON);
    }
    {
        props_and_embeddings_t paes = {
            {0, 0, bidi_prop_t::AL, false},
            {0, 0, bidi_prop_t::BN, false},
            {0, 0, bidi_prop_t::NSM, false},
            {0, 0, bidi_prop_t::BN, false},
            {0, 0, bidi_prop_t::NSM, false},
        };

        auto runs = find_all_runs(paes.begin(), paes.end());
        auto run_sequences = find_run_sequences(paes, runs);
        EXPECT_EQ(run_sequences.size(), 1u);

        find_sos_eos(run_sequences, 1); // 1 implies sos==R
        w1(run_sequences[0]);

        auto it = run_sequences[0].begin();
        EXPECT_EQ(
            std::distance(run_sequences[0].begin(), run_sequences[0].end()), 5);
        EXPECT_EQ(it->prop_, bidi_prop_t::AL);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::BN);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::AL);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::BN);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::AL);
    }
    {
        props_and_embeddings_t paes = {
            {0, 0, bidi_prop_t::BN, false},
            {0, 0, bidi_prop_t::NSM, false},
        };

        auto runs = find_all_runs(paes.begin(), paes.end());
        auto run_sequences = find_run_sequences(paes, runs);
        EXPECT_EQ(run_sequences.size(), 1u);

        find_sos_eos(run_sequences, 1); // 1 implies sos==R
        w1(run_sequences[0]);

        auto it = run_sequences[0].begin();
        EXPECT_EQ(
            std::distance(run_sequences[0].begin(), run_sequences[0].end()), 2);
        EXPECT_EQ(it->prop_, bidi_prop_t::BN);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::R);
    }

    // W2
    {
        props_and_embeddings_t paes = {
            {0, 0, bidi_prop_t::AL, false},
            {0, 0, bidi_prop_t::EN, false},
        };

        auto runs = find_all_runs(paes.begin(), paes.end());
        auto run_sequences = find_run_sequences(paes, runs);
        EXPECT_EQ(run_sequences.size(), 1u);

        find_sos_eos(run_sequences, 1); // 1 implies sos==R
        w2(run_sequences[0]);

        auto it = run_sequences[0].begin();
        EXPECT_EQ(
            std::distance(run_sequences[0].begin(), run_sequences[0].end()), 2);
        EXPECT_EQ(it->prop_, bidi_prop_t::AL);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::AN);
    }
    {
        props_and_embeddings_t paes = {
            {0, 0, bidi_prop_t::AL, false},
            {0, 0, bidi_prop_t::B, false},
            {0, 0, bidi_prop_t::EN, false},
        };

        auto runs = find_all_runs(paes.begin(), paes.end());
        auto run_sequences = find_run_sequences(paes, runs);
        EXPECT_EQ(run_sequences.size(), 1u);

        find_sos_eos(run_sequences, 1); // 1 implies sos==R
        w2(run_sequences[0]);

        auto it = run_sequences[0].begin();
        EXPECT_EQ(
            std::distance(run_sequences[0].begin(), run_sequences[0].end()), 3);
        EXPECT_EQ(it->prop_, bidi_prop_t::AL);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::B);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::AN);
    }
    {
        props_and_embeddings_t paes = {
            {0, 0, bidi_prop_t::B, false},
            {0, 0, bidi_prop_t::EN, false},
        };

        auto runs = find_all_runs(paes.begin(), paes.end());
        auto run_sequences = find_run_sequences(paes, runs);
        EXPECT_EQ(run_sequences.size(), 1u);

        find_sos_eos(run_sequences, 1); // 1 implies sos==R
        w2(run_sequences[0]);

        auto it = run_sequences[0].begin();
        EXPECT_EQ(
            std::distance(run_sequences[0].begin(), run_sequences[0].end()), 2);
        EXPECT_EQ(it->prop_, bidi_prop_t::B);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::EN);
    }
    {
        props_and_embeddings_t paes = {
            {0, 0, bidi_prop_t::L, false},
            {0, 0, bidi_prop_t::B, false},
            {0, 0, bidi_prop_t::EN, false},
        };

        auto runs = find_all_runs(paes.begin(), paes.end());
        auto run_sequences = find_run_sequences(paes, runs);
        EXPECT_EQ(run_sequences.size(), 1u);

        find_sos_eos(run_sequences, 1); // 1 implies sos==R
        w2(run_sequences[0]);

        auto it = run_sequences[0].begin();
        EXPECT_EQ(
            std::distance(run_sequences[0].begin(), run_sequences[0].end()), 3);
        EXPECT_EQ(it->prop_, bidi_prop_t::L);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::B);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::EN);
    }
    {
        props_and_embeddings_t paes = {
            {0, 0, bidi_prop_t::R, false},
            {0, 0, bidi_prop_t::B, false},
            {0, 0, bidi_prop_t::EN, false},
        };

        auto runs = find_all_runs(paes.begin(), paes.end());
        auto run_sequences = find_run_sequences(paes, runs);
        EXPECT_EQ(run_sequences.size(), 1u);

        find_sos_eos(run_sequences, 1); // 1 implies sos==R
        w2(run_sequences[0]);

        auto it = run_sequences[0].begin();
        EXPECT_EQ(
            std::distance(run_sequences[0].begin(), run_sequences[0].end()), 3);
        EXPECT_EQ(it->prop_, bidi_prop_t::R);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::B);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::EN);
    }

    // W4
    {
        props_and_embeddings_t paes = {
            {0, 0, bidi_prop_t::EN, false},
            {0, 0, bidi_prop_t::ES, false},
            {0, 0, bidi_prop_t::EN, false},
        };

        auto runs = find_all_runs(paes.begin(), paes.end());
        auto run_sequences = find_run_sequences(paes, runs);
        EXPECT_EQ(run_sequences.size(), 1u);

        find_sos_eos(run_sequences, 1); // 1 implies sos==R
        w4(run_sequences[0]);

        auto it = run_sequences[0].begin();
        EXPECT_EQ(
            std::distance(run_sequences[0].begin(), run_sequences[0].end()), 3);
        EXPECT_EQ(it->prop_, bidi_prop_t::EN);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::EN);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::EN);
    }
    {
        props_and_embeddings_t paes = {
            {0, 0, bidi_prop_t::EN, false},
            {0, 0, bidi_prop_t::CS, false},
            {0, 0, bidi_prop_t::EN, false},
        };

        auto runs = find_all_runs(paes.begin(), paes.end());
        auto run_sequences = find_run_sequences(paes, runs);
        EXPECT_EQ(run_sequences.size(), 1u);

        find_sos_eos(run_sequences, 1); // 1 implies sos==R
        w4(run_sequences[0]);

        auto it = run_sequences[0].begin();
        EXPECT_EQ(
            std::distance(run_sequences[0].begin(), run_sequences[0].end()), 3);
        EXPECT_EQ(it->prop_, bidi_prop_t::EN);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::EN);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::EN);
    }
    {
        props_and_embeddings_t paes = {
            {0, 0, bidi_prop_t::AN, false},
            {0, 0, bidi_prop_t::CS, false},
            {0, 0, bidi_prop_t::AN, false},
        };

        auto runs = find_all_runs(paes.begin(), paes.end());
        auto run_sequences = find_run_sequences(paes, runs);
        EXPECT_EQ(run_sequences.size(), 1u);

        find_sos_eos(run_sequences, 1); // 1 implies sos==R
        w4(run_sequences[0]);

        auto it = run_sequences[0].begin();
        EXPECT_EQ(
            std::distance(run_sequences[0].begin(), run_sequences[0].end()), 3);
        EXPECT_EQ(it->prop_, bidi_prop_t::AN);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::AN);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::AN);
    }
    {
        props_and_embeddings_t paes = {
            {0, 0, bidi_prop_t::BN, false},
            {0, 0, bidi_prop_t::EN, false},
            {0, 0, bidi_prop_t::BN, false},
            {0, 0, bidi_prop_t::BN, false},
            {0, 0, bidi_prop_t::ES, false},
            {0, 0, bidi_prop_t::BN, false},
            {0, 0, bidi_prop_t::EN, false},
            {0, 0, bidi_prop_t::BN, false},
        };

        auto runs = find_all_runs(paes.begin(), paes.end());
        auto run_sequences = find_run_sequences(paes, runs);
        EXPECT_EQ(run_sequences.size(), 1u);

        find_sos_eos(run_sequences, 1); // 1 implies sos==R
        w4(run_sequences[0]);

        auto it = run_sequences[0].begin();
        EXPECT_EQ(
            std::distance(run_sequences[0].begin(), run_sequences[0].end()), 8);
        EXPECT_EQ(it->prop_, bidi_prop_t::BN);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::EN);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::BN);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::BN);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::EN);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::BN);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::EN);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::BN);
    }
    {
        props_and_embeddings_t paes = {
            {0, 0, bidi_prop_t::BN, false},
            {0, 0, bidi_prop_t::EN, false},
            {0, 0, bidi_prop_t::BN, false},
            {0, 0, bidi_prop_t::CS, false},
            {0, 0, bidi_prop_t::BN, false},
            {0, 0, bidi_prop_t::EN, false},
            {0, 0, bidi_prop_t::BN, false},
        };

        auto runs = find_all_runs(paes.begin(), paes.end());
        auto run_sequences = find_run_sequences(paes, runs);
        EXPECT_EQ(run_sequences.size(), 1u);

        find_sos_eos(run_sequences, 1); // 1 implies sos==R
        w4(run_sequences[0]);

        auto it = run_sequences[0].begin();
        EXPECT_EQ(
            std::distance(run_sequences[0].begin(), run_sequences[0].end()), 7);
        EXPECT_EQ(it->prop_, bidi_prop_t::BN);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::EN);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::BN);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::EN);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::BN);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::EN);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::BN);
    }
    {
        props_and_embeddings_t paes = {
            {0, 0, bidi_prop_t::BN, false},
            {0, 0, bidi_prop_t::BN, false},
            {0, 0, bidi_prop_t::AN, false},
            {0, 0, bidi_prop_t::BN, false},
            {0, 0, bidi_prop_t::CS, false},
            {0, 0, bidi_prop_t::BN, false},
            {0, 0, bidi_prop_t::AN, false},
            {0, 0, bidi_prop_t::BN, false},
        };

        auto runs = find_all_runs(paes.begin(), paes.end());
        auto run_sequences = find_run_sequences(paes, runs);
        EXPECT_EQ(run_sequences.size(), 1u);

        find_sos_eos(run_sequences, 1); // 1 implies sos==R
        w4(run_sequences[0]);

        auto it = run_sequences[0].begin();
        EXPECT_EQ(
            std::distance(run_sequences[0].begin(), run_sequences[0].end()), 8);
        EXPECT_EQ(it->prop_, bidi_prop_t::BN);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::BN);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::AN);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::BN);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::AN);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::BN);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::AN);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::BN);
    }

    // W5
    {
        props_and_embeddings_t paes = {
            {0, 0, bidi_prop_t::ET, false},
            {0, 0, bidi_prop_t::ET, false},
            {0, 0, bidi_prop_t::EN, false},
        };

        auto runs = find_all_runs(paes.begin(), paes.end());
        auto run_sequences = find_run_sequences(paes, runs);
        EXPECT_EQ(run_sequences.size(), 1u);

        find_sos_eos(run_sequences, 1); // 1 implies sos==R
        w5(run_sequences[0]);

        auto it = run_sequences[0].begin();
        EXPECT_EQ(
            std::distance(run_sequences[0].begin(), run_sequences[0].end()), 3);
        EXPECT_EQ(it->prop_, bidi_prop_t::EN);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::EN);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::EN);
    }
    {
        props_and_embeddings_t paes = {
            {0, 0, bidi_prop_t::EN, false},
            {0, 0, bidi_prop_t::ET, false},
            {0, 0, bidi_prop_t::ET, false},
        };

        auto runs = find_all_runs(paes.begin(), paes.end());
        auto run_sequences = find_run_sequences(paes, runs);
        EXPECT_EQ(run_sequences.size(), 1u);

        find_sos_eos(run_sequences, 1); // 1 implies sos==R
        w5(run_sequences[0]);

        auto it = run_sequences[0].begin();
        EXPECT_EQ(
            std::distance(run_sequences[0].begin(), run_sequences[0].end()), 3);
        EXPECT_EQ(it->prop_, bidi_prop_t::EN);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::EN);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::EN);
    }
    {
        props_and_embeddings_t paes = {
            {0, 0, bidi_prop_t::AN, false},
            {0, 0, bidi_prop_t::ET, false},
            {0, 0, bidi_prop_t::EN, false},
        };

        auto runs = find_all_runs(paes.begin(), paes.end());
        auto run_sequences = find_run_sequences(paes, runs);
        EXPECT_EQ(run_sequences.size(), 1u);

        find_sos_eos(run_sequences, 1); // 1 implies sos==R
        w5(run_sequences[0]);

        auto it = run_sequences[0].begin();
        EXPECT_EQ(
            std::distance(run_sequences[0].begin(), run_sequences[0].end()), 3);
        EXPECT_EQ(it->prop_, bidi_prop_t::AN);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::EN);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::EN);
    }
    {
        props_and_embeddings_t paes = {
            {0, 0, bidi_prop_t::ET, false},
            {0, 0, bidi_prop_t::AN, false},
            {0, 0, bidi_prop_t::EN, false},
        };

        auto runs = find_all_runs(paes.begin(), paes.end());
        auto run_sequences = find_run_sequences(paes, runs);
        EXPECT_EQ(run_sequences.size(), 1u);

        find_sos_eos(run_sequences, 1); // 1 implies sos==R
        w5(run_sequences[0]);

        auto it = run_sequences[0].begin();
        EXPECT_EQ(
            std::distance(run_sequences[0].begin(), run_sequences[0].end()), 3);
        EXPECT_EQ(it->prop_, bidi_prop_t::ET);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::AN);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::EN);
    }
    {
        props_and_embeddings_t paes = {
            {0, 0, bidi_prop_t::EN, false},
            {0, 0, bidi_prop_t::AN, false},
            {0, 0, bidi_prop_t::ET, false},
        };

        auto runs = find_all_runs(paes.begin(), paes.end());
        auto run_sequences = find_run_sequences(paes, runs);
        EXPECT_EQ(run_sequences.size(), 1u);

        find_sos_eos(run_sequences, 1); // 1 implies sos==R
        w5(run_sequences[0]);

        auto it = run_sequences[0].begin();
        EXPECT_EQ(
            std::distance(run_sequences[0].begin(), run_sequences[0].end()), 3);
        EXPECT_EQ(it->prop_, bidi_prop_t::EN);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::AN);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::ET);
    }
    {
        props_and_embeddings_t paes = {
            {0, 0, bidi_prop_t::EN, false},
            {0, 0, bidi_prop_t::EN, false},
            {0, 0, bidi_prop_t::ET, false},
        };

        auto runs = find_all_runs(paes.begin(), paes.end());
        auto run_sequences = find_run_sequences(paes, runs);
        EXPECT_EQ(run_sequences.size(), 1u);

        find_sos_eos(run_sequences, 1); // 1 implies sos==R
        w5(run_sequences[0]);

        auto it = run_sequences[0].begin();
        EXPECT_EQ(
            std::distance(run_sequences[0].begin(), run_sequences[0].end()), 3);
        EXPECT_EQ(it->prop_, bidi_prop_t::EN);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::EN);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::EN);
    }
    {
        props_and_embeddings_t paes = {
            {0, 0, bidi_prop_t::BN, false},
            {0, 0, bidi_prop_t::ET, false},
            {0, 0, bidi_prop_t::ET, false},
            {0, 0, bidi_prop_t::BN, false},
            {0, 0, bidi_prop_t::EN, false},
        };

        auto runs = find_all_runs(paes.begin(), paes.end());
        auto run_sequences = find_run_sequences(paes, runs);
        EXPECT_EQ(run_sequences.size(), 1u);

        find_sos_eos(run_sequences, 1); // 1 implies sos==R
        w5(run_sequences[0]);

        auto it = run_sequences[0].begin();
        EXPECT_EQ(
            std::distance(run_sequences[0].begin(), run_sequences[0].end()), 5);
        EXPECT_EQ(it->prop_, bidi_prop_t::EN);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::EN);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::EN);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::EN);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::EN);
    }
    {
        props_and_embeddings_t paes = {
            {0, 0, bidi_prop_t::BN, false},
            {0, 0, bidi_prop_t::EN, false},
            {0, 0, bidi_prop_t::BN, false},
            {0, 0, bidi_prop_t::ET, false},
            {0, 0, bidi_prop_t::ET, false},
            {0, 0, bidi_prop_t::BN, false},
        };

        auto runs = find_all_runs(paes.begin(), paes.end());
        auto run_sequences = find_run_sequences(paes, runs);
        EXPECT_EQ(run_sequences.size(), 1u);

        find_sos_eos(run_sequences, 1); // 1 implies sos==R
        w5(run_sequences[0]);

        auto it = run_sequences[0].begin();
        EXPECT_EQ(
            std::distance(run_sequences[0].begin(), run_sequences[0].end()), 6);
        EXPECT_EQ(it->prop_, bidi_prop_t::EN);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::EN);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::EN);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::EN);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::EN);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::EN);
    }

    // W6
    {
        props_and_embeddings_t paes = {
            {0, 0, bidi_prop_t::AN, false},
            {0, 0, bidi_prop_t::ET, false},
        };

        auto runs = find_all_runs(paes.begin(), paes.end());
        auto run_sequences = find_run_sequences(paes, runs);
        EXPECT_EQ(run_sequences.size(), 1u);

        find_sos_eos(run_sequences, 1); // 1 implies sos==R
        w6(run_sequences[0]);

        auto it = run_sequences[0].begin();
        EXPECT_EQ(
            std::distance(run_sequences[0].begin(), run_sequences[0].end()), 2);
        EXPECT_EQ(it->prop_, bidi_prop_t::AN);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::ON);
    }
    {
        props_and_embeddings_t paes = {
            {0, 0, bidi_prop_t::L, false},
            {0, 0, bidi_prop_t::ES, false},
            {0, 0, bidi_prop_t::EN, false},
        };

        auto runs = find_all_runs(paes.begin(), paes.end());
        auto run_sequences = find_run_sequences(paes, runs);
        EXPECT_EQ(run_sequences.size(), 1u);

        find_sos_eos(run_sequences, 1); // 1 implies sos==R
        w6(run_sequences[0]);

        auto it = run_sequences[0].begin();
        EXPECT_EQ(
            std::distance(run_sequences[0].begin(), run_sequences[0].end()), 3);
        EXPECT_EQ(it->prop_, bidi_prop_t::L);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::ON);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::EN);
    }
    {
        props_and_embeddings_t paes = {
            {0, 0, bidi_prop_t::EN, false},
            {0, 0, bidi_prop_t::CS, false},
            {0, 0, bidi_prop_t::AN, false},
        };

        auto runs = find_all_runs(paes.begin(), paes.end());
        auto run_sequences = find_run_sequences(paes, runs);
        EXPECT_EQ(run_sequences.size(), 1u);

        find_sos_eos(run_sequences, 1); // 1 implies sos==R
        w6(run_sequences[0]);

        auto it = run_sequences[0].begin();
        EXPECT_EQ(
            std::distance(run_sequences[0].begin(), run_sequences[0].end()), 3);
        EXPECT_EQ(it->prop_, bidi_prop_t::EN);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::ON);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::AN);
    }
    {
        props_and_embeddings_t paes = {
            {0, 0, bidi_prop_t::ET, false},
            {0, 0, bidi_prop_t::AN, false},
        };

        auto runs = find_all_runs(paes.begin(), paes.end());
        auto run_sequences = find_run_sequences(paes, runs);
        EXPECT_EQ(run_sequences.size(), 1u);

        find_sos_eos(run_sequences, 1); // 1 implies sos==R
        w6(run_sequences[0]);

        auto it = run_sequences[0].begin();
        EXPECT_EQ(
            std::distance(run_sequences[0].begin(), run_sequences[0].end()), 2);
        EXPECT_EQ(it->prop_, bidi_prop_t::ON);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::AN);
    }

    {
        props_and_embeddings_t paes = {
            {0, 0, bidi_prop_t::BN, false},
            {0, 0, bidi_prop_t::AN, false},
            {0, 0, bidi_prop_t::BN, false},
            {0, 0, bidi_prop_t::ET, false},
        };

        auto runs = find_all_runs(paes.begin(), paes.end());
        auto run_sequences = find_run_sequences(paes, runs);
        EXPECT_EQ(run_sequences.size(), 1u);

        find_sos_eos(run_sequences, 1); // 1 implies sos==R
        w6(run_sequences[0]);

        auto it = run_sequences[0].begin();
        EXPECT_EQ(
            std::distance(run_sequences[0].begin(), run_sequences[0].end()), 4);
        EXPECT_EQ(it->prop_, bidi_prop_t::BN);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::AN);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::ON);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::ON);
    }
    {
        props_and_embeddings_t paes = {
            {0, 0, bidi_prop_t::L, false},
            {0, 0, bidi_prop_t::ES, false},
            {0, 0, bidi_prop_t::BN, false},
            {0, 0, bidi_prop_t::EN, false},
            {0, 0, bidi_prop_t::BN, false},
        };

        auto runs = find_all_runs(paes.begin(), paes.end());
        auto run_sequences = find_run_sequences(paes, runs);
        EXPECT_EQ(run_sequences.size(), 1u);

        find_sos_eos(run_sequences, 1); // 1 implies sos==R
        w6(run_sequences[0]);

        auto it = run_sequences[0].begin();
        EXPECT_EQ(
            std::distance(run_sequences[0].begin(), run_sequences[0].end()), 5);
        EXPECT_EQ(it->prop_, bidi_prop_t::L);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::ON);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::ON);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::EN);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::BN);
    }
    {
        props_and_embeddings_t paes = {
            {0, 0, bidi_prop_t::EN, false},
            {0, 0, bidi_prop_t::BN, false},
            {0, 0, bidi_prop_t::CS, false},
            {0, 0, bidi_prop_t::AN, false},
        };

        auto runs = find_all_runs(paes.begin(), paes.end());
        auto run_sequences = find_run_sequences(paes, runs);
        EXPECT_EQ(run_sequences.size(), 1u);

        find_sos_eos(run_sequences, 1); // 1 implies sos==R
        w6(run_sequences[0]);

        auto it = run_sequences[0].begin();
        EXPECT_EQ(
            std::distance(run_sequences[0].begin(), run_sequences[0].end()), 4);
        EXPECT_EQ(it->prop_, bidi_prop_t::EN);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::ON);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::ON);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::AN);
    }
    {
        props_and_embeddings_t paes = {
            {0, 0, bidi_prop_t::BN, false},
            {0, 0, bidi_prop_t::ET, false},
            {0, 0, bidi_prop_t::AN, false},
        };

        auto runs = find_all_runs(paes.begin(), paes.end());
        auto run_sequences = find_run_sequences(paes, runs);
        EXPECT_EQ(run_sequences.size(), 1u);

        find_sos_eos(run_sequences, 1); // 1 implies sos==R
        w6(run_sequences[0]);

        auto it = run_sequences[0].begin();
        EXPECT_EQ(
            std::distance(run_sequences[0].begin(), run_sequences[0].end()), 3);
        EXPECT_EQ(it->prop_, bidi_prop_t::ON);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::ON);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::AN);
    }
    {
        props_and_embeddings_t paes = {
            {0, 0, bidi_prop_t::ET, false},
            {0, 0, bidi_prop_t::BN, false},
            {0, 0, bidi_prop_t::AN, false},
            {0, 0, bidi_prop_t::BN, false},
        };

        auto runs = find_all_runs(paes.begin(), paes.end());
        auto run_sequences = find_run_sequences(paes, runs);
        EXPECT_EQ(run_sequences.size(), 1u);

        find_sos_eos(run_sequences, 1); // 1 implies sos==R
        w6(run_sequences[0]);

        auto it = run_sequences[0].begin();
        EXPECT_EQ(
            std::distance(run_sequences[0].begin(), run_sequences[0].end()), 4);
        EXPECT_EQ(it->prop_, bidi_prop_t::ON);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::ON);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::AN);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::BN);
    }

    // W7
    {
        props_and_embeddings_t paes = {
            {0, 0, bidi_prop_t::L, false},
            {0, 0, bidi_prop_t::B, false},
            {0, 0, bidi_prop_t::EN, false},
        };

        auto runs = find_all_runs(paes.begin(), paes.end());
        auto run_sequences = find_run_sequences(paes, runs);
        EXPECT_EQ(run_sequences.size(), 1u);

        find_sos_eos(run_sequences, 1); // 1 implies sos==R
        w7(run_sequences[0]);

        auto it = run_sequences[0].begin();
        EXPECT_EQ(
            std::distance(run_sequences[0].begin(), run_sequences[0].end()), 3);
        EXPECT_EQ(it->prop_, bidi_prop_t::L);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::B);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::L);
    }
    {
        props_and_embeddings_t paes = {
            {0, 0, bidi_prop_t::R, false},
            {0, 0, bidi_prop_t::B, false},
            {0, 0, bidi_prop_t::EN, false},
        };

        auto runs = find_all_runs(paes.begin(), paes.end());
        auto run_sequences = find_run_sequences(paes, runs);
        EXPECT_EQ(run_sequences.size(), 1u);

        find_sos_eos(run_sequences, 1); // 1 implies sos==R
        w7(run_sequences[0]);

        auto it = run_sequences[0].begin();
        EXPECT_EQ(
            std::distance(run_sequences[0].begin(), run_sequences[0].end()), 3);
        EXPECT_EQ(it->prop_, bidi_prop_t::R);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::B);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::EN);
    }
    {
        props_and_embeddings_t paes = {
            {0, 0, bidi_prop_t::L, false},
            {0, 0, bidi_prop_t::BN, false},
            {0, 0, bidi_prop_t::B, false},
            {0, 0, bidi_prop_t::EN, false},
        };

        auto runs = find_all_runs(paes.begin(), paes.end());
        auto run_sequences = find_run_sequences(paes, runs);
        EXPECT_EQ(run_sequences.size(), 1u);

        find_sos_eos(run_sequences, 1); // 1 implies sos==R
        w7(run_sequences[0]);

        auto it = run_sequences[0].begin();
        EXPECT_EQ(
            std::distance(run_sequences[0].begin(), run_sequences[0].end()), 4);
        EXPECT_EQ(it->prop_, bidi_prop_t::L);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::BN);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::B);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::L);
    }
    {
        props_and_embeddings_t paes = {
            {0, 0, bidi_prop_t::R, false},
            {0, 0, bidi_prop_t::B, false},
            {0, 0, bidi_prop_t::BN, false},
            {0, 0, bidi_prop_t::EN, false},
        };

        auto runs = find_all_runs(paes.begin(), paes.end());
        auto run_sequences = find_run_sequences(paes, runs);
        EXPECT_EQ(run_sequences.size(), 1u);

        find_sos_eos(run_sequences, 1); // 1 implies sos==R
        w7(run_sequences[0]);

        auto it = run_sequences[0].begin();
        EXPECT_EQ(
            std::distance(run_sequences[0].begin(), run_sequences[0].end()), 4);
        EXPECT_EQ(it->prop_, bidi_prop_t::R);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::B);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::BN);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::EN);
    }
}

TEST(detail_bidi, find_bracket_pairs_)
{
    {
        props_and_embeddings_t paes = {
            {'a', 0, bidi_prop_t::ON, false},
            {')', 0, bidi_prop_t::ON, false},
            {'b', 0, bidi_prop_t::ON, false},
            {'(', 0, bidi_prop_t::ON, false},
            {'c', 0, bidi_prop_t::ON, false},
        };

        auto runs = find_all_runs(paes.begin(), paes.end());
        auto run_sequences = find_run_sequences(paes, runs);
        EXPECT_EQ(run_sequences.size(), 1u);

        auto const bracket_pairs = find_bracket_pairs(run_sequences[0]);

        EXPECT_EQ(std::distance(bracket_pairs.begin(), bracket_pairs.end()), 0);
    }
    {
        props_and_embeddings_t paes = {
            {'a', 0, bidi_prop_t::ON, false},
            {')', 0, bidi_prop_t::ON, false},
            {'b', 0, bidi_prop_t::ON, false},
            {']', 0, bidi_prop_t::ON, false},
            {'c', 0, bidi_prop_t::ON, false},
        };

        auto runs = find_all_runs(paes.begin(), paes.end());
        auto run_sequences = find_run_sequences(paes, runs);
        EXPECT_EQ(run_sequences.size(), 1u);

        auto const bracket_pairs = find_bracket_pairs(run_sequences[0]);

        EXPECT_EQ(std::distance(bracket_pairs.begin(), bracket_pairs.end()), 0);
    }
    {
        props_and_embeddings_t paes = {
            {'a', 0, bidi_prop_t::ON, false},
            {'(', 0, bidi_prop_t::ON, false},
            {'b', 0, bidi_prop_t::ON, false},
            {')', 0, bidi_prop_t::ON, false},
            {'c', 0, bidi_prop_t::ON, false},
        };

        auto runs = find_all_runs(paes.begin(), paes.end());
        auto run_sequences = find_run_sequences(paes, runs);
        EXPECT_EQ(run_sequences.size(), 1u);

        auto const bracket_pairs = find_bracket_pairs(run_sequences[0]);

        auto it = bracket_pairs.begin();
        EXPECT_EQ(std::distance(bracket_pairs.begin(), bracket_pairs.end()), 1);
        EXPECT_EQ(it->first_.base() - paes.begin(), 1);
        EXPECT_EQ(it->last_.base() - paes.begin(), 3);
    }
    {
        props_and_embeddings_t paes = {
            {'a', 0, bidi_prop_t::ON, false},
            {'(', 0, bidi_prop_t::ON, false},
            {'b', 0, bidi_prop_t::ON, false},
            {'[', 0, bidi_prop_t::ON, false},
            {'c', 0, bidi_prop_t::ON, false},
            {')', 0, bidi_prop_t::ON, false},
            {'d', 0, bidi_prop_t::ON, false},
            {']', 0, bidi_prop_t::ON, false},
        };

        auto runs = find_all_runs(paes.begin(), paes.end());
        auto run_sequences = find_run_sequences(paes, runs);
        EXPECT_EQ(run_sequences.size(), 1u);

        auto const bracket_pairs = find_bracket_pairs(run_sequences[0]);

        auto it = bracket_pairs.begin();
        EXPECT_EQ(std::distance(bracket_pairs.begin(), bracket_pairs.end()), 1);
        EXPECT_EQ(it->first_.base() - paes.begin(), 1);
        EXPECT_EQ(it->last_.base() - paes.begin(), 5);
    }
    {
        props_and_embeddings_t paes = {
            {'a', 0, bidi_prop_t::ON, false},
            {'(', 0, bidi_prop_t::ON, false},
            {'b', 0, bidi_prop_t::ON, false},
            {']', 0, bidi_prop_t::ON, false},
            {'c', 0, bidi_prop_t::ON, false},
            {')', 0, bidi_prop_t::ON, false},
            {'d', 0, bidi_prop_t::ON, false},
        };

        auto runs = find_all_runs(paes.begin(), paes.end());
        auto run_sequences = find_run_sequences(paes, runs);
        EXPECT_EQ(run_sequences.size(), 1u);

        auto const bracket_pairs = find_bracket_pairs(run_sequences[0]);

        auto it = bracket_pairs.begin();
        EXPECT_EQ(std::distance(bracket_pairs.begin(), bracket_pairs.end()), 1);
        EXPECT_EQ(it->first_.base() - paes.begin(), 1);
        EXPECT_EQ(it->last_.base() - paes.begin(), 5);
    }
    {
        props_and_embeddings_t paes = {
            {'a', 0, bidi_prop_t::ON, false},
            {'(', 0, bidi_prop_t::ON, false},
            {'b', 0, bidi_prop_t::ON, false},
            {')', 0, bidi_prop_t::ON, false},
            {'c', 0, bidi_prop_t::ON, false},
            {')', 0, bidi_prop_t::ON, false},
            {'d', 0, bidi_prop_t::ON, false},
        };

        auto runs = find_all_runs(paes.begin(), paes.end());
        auto run_sequences = find_run_sequences(paes, runs);
        EXPECT_EQ(run_sequences.size(), 1u);

        auto const bracket_pairs = find_bracket_pairs(run_sequences[0]);

        auto it = bracket_pairs.begin();
        EXPECT_EQ(std::distance(bracket_pairs.begin(), bracket_pairs.end()), 1);
        EXPECT_EQ(it->first_.base() - paes.begin(), 1);
        EXPECT_EQ(it->last_.base() - paes.begin(), 3);
    }
    {
        props_and_embeddings_t paes = {
            {'a', 0, bidi_prop_t::ON, false},
            {'(', 0, bidi_prop_t::ON, false},
            {'b', 0, bidi_prop_t::ON, false},
            {'(', 0, bidi_prop_t::ON, false},
            {'c', 0, bidi_prop_t::ON, false},
            {')', 0, bidi_prop_t::ON, false},
            {'d', 0, bidi_prop_t::ON, false},
        };

        auto runs = find_all_runs(paes.begin(), paes.end());
        auto run_sequences = find_run_sequences(paes, runs);
        EXPECT_EQ(run_sequences.size(), 1u);

        auto const bracket_pairs = find_bracket_pairs(run_sequences[0]);

        auto it = bracket_pairs.begin();
        EXPECT_EQ(std::distance(bracket_pairs.begin(), bracket_pairs.end()), 1);
        EXPECT_EQ(it->first_.base() - paes.begin(), 3);
        EXPECT_EQ(it->last_.base() - paes.begin(), 5);
    }
    {
        props_and_embeddings_t paes = {
            {'a', 0, bidi_prop_t::ON, false},
            {'(', 0, bidi_prop_t::ON, false},
            {'b', 0, bidi_prop_t::ON, false},
            {'(', 0, bidi_prop_t::ON, false},
            {'c', 0, bidi_prop_t::ON, false},
            {')', 0, bidi_prop_t::ON, false},
            {'d', 0, bidi_prop_t::ON, false},
            {')', 0, bidi_prop_t::ON, false},
        };

        auto runs = find_all_runs(paes.begin(), paes.end());
        auto run_sequences = find_run_sequences(paes, runs);
        EXPECT_EQ(run_sequences.size(), 1u);

        auto const bracket_pairs = find_bracket_pairs(run_sequences[0]);

        auto it = bracket_pairs.begin();
        EXPECT_EQ(std::distance(bracket_pairs.begin(), bracket_pairs.end()), 2);
        EXPECT_EQ(it->first_.base() - paes.begin(), 1);
        EXPECT_EQ(it->last_.base() - paes.begin(), 7);
        ++it;
        EXPECT_EQ(it->first_.base() - paes.begin(), 3);
        EXPECT_EQ(it->last_.base() - paes.begin(), 5);
    }
    {
        props_and_embeddings_t paes = {
            {'a', 0, bidi_prop_t::ON, false},
            {'(', 0, bidi_prop_t::ON, false},
            {'b', 0, bidi_prop_t::ON, false},
            {'{', 0, bidi_prop_t::ON, false},
            {'c', 0, bidi_prop_t::ON, false},
            {'}', 0, bidi_prop_t::ON, false},
            {'d', 0, bidi_prop_t::ON, false},
            {')', 0, bidi_prop_t::ON, false},
        };

        auto runs = find_all_runs(paes.begin(), paes.end());
        auto run_sequences = find_run_sequences(paes, runs);
        EXPECT_EQ(run_sequences.size(), 1u);

        auto const bracket_pairs = find_bracket_pairs(run_sequences[0]);

        auto it = bracket_pairs.begin();
        EXPECT_EQ(std::distance(bracket_pairs.begin(), bracket_pairs.end()), 2);
        EXPECT_EQ(it->first_.base() - paes.begin(), 1);
        EXPECT_EQ(it->last_.base() - paes.begin(), 7);
        ++it;
        EXPECT_EQ(it->first_.base() - paes.begin(), 3);
        EXPECT_EQ(it->last_.base() - paes.begin(), 5);
    }
}

TEST(detail_bidi, n0_)
{
    {
        props_and_embeddings_t paes = {
            {'A', 1, bidi_prop_t::R, false},
            {'B', 1, bidi_prop_t::R, false},
            {'(', 1, bidi_prop_t::ON, false},
            {'C', 1, bidi_prop_t::R, false},
            {'D', 1, bidi_prop_t::R, false},
            {'[', 1, bidi_prop_t::ON, false},
            {'&', 1, bidi_prop_t::ON, false},
            {'e', 1, bidi_prop_t::L, false},
            {'f', 1, bidi_prop_t::L, false},
            {']', 1, bidi_prop_t::ON, false},
            {'!', 1, bidi_prop_t::ON, false},
            {')', 1, bidi_prop_t::ON, false},
            {'g', 1, bidi_prop_t::L, false},
            {'h', 1, bidi_prop_t::L, false},
        };

        auto runs = find_all_runs(paes.begin(), paes.end());
        auto run_sequences = find_run_sequences(paes, runs);
        EXPECT_EQ(run_sequences.size(), 1u);

        auto const bracket_pairs = find_bracket_pairs(run_sequences[0]);
        n0(run_sequences[0], bracket_pairs);

        auto it = run_sequences[0].begin();
        EXPECT_EQ(
            std::distance(run_sequences[0].begin(), run_sequences[0].end()),
            14);
        EXPECT_EQ(it->prop_, bidi_prop_t::R);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::R);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::R);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::R);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::R);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::R);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::ON);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::L);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::L);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::R);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::ON);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::R);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::L);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::L);
    }
    {
        props_and_embeddings_t paes = {
            {'s', 1, bidi_prop_t::L, false},
            {'m', 1, bidi_prop_t::L, false},
            {'i', 1, bidi_prop_t::L, false},
            {'t', 1, bidi_prop_t::L, false},
            {'h', 1, bidi_prop_t::L, false},
            {' ', 1, bidi_prop_t::WS, false},
            {'(', 1, bidi_prop_t::ON, false},
            {'f', 1, bidi_prop_t::L, false},
            {'a', 1, bidi_prop_t::L, false},
            {'b', 1, bidi_prop_t::L, false},
            {'r', 1, bidi_prop_t::L, false},
            {'i', 1, bidi_prop_t::L, false},
            {'k', 1, bidi_prop_t::L, false},
            {'a', 1, bidi_prop_t::L, false},
            {'m', 1, bidi_prop_t::L, false},
            {' ', 1, bidi_prop_t::WS, false},
            {'A', 1, bidi_prop_t::R, false},
            {'R', 1, bidi_prop_t::R, false},
            {'A', 1, bidi_prop_t::R, false},
            {'B', 1, bidi_prop_t::R, false},
            {'I', 1, bidi_prop_t::R, false},
            {'C', 1, bidi_prop_t::R, false},
            {')', 1, bidi_prop_t::ON, false},
            {' ', 1, bidi_prop_t::WS, false},
            {'H', 1, bidi_prop_t::R, false},
            {'E', 1, bidi_prop_t::R, false},
            {'B', 1, bidi_prop_t::R, false},
            {'R', 1, bidi_prop_t::R, false},
            {'E', 1, bidi_prop_t::R, false},
            {'W', 1, bidi_prop_t::R, false},
        };

        auto runs = find_all_runs(paes.begin(), paes.end());
        auto run_sequences = find_run_sequences(paes, runs);
        EXPECT_EQ(run_sequences.size(), 1u);

        auto const bracket_pairs = find_bracket_pairs(run_sequences[0]);
        n0(run_sequences[0], bracket_pairs);

        auto it = run_sequences[0].begin();
        EXPECT_EQ(
            std::distance(run_sequences[0].begin(), run_sequences[0].end()),
            30);
        EXPECT_EQ(it->prop_, bidi_prop_t::L);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::L);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::L);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::L);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::L);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::WS);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::R);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::L);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::L);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::L);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::L);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::L);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::L);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::L);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::L);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::WS);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::R);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::R);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::R);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::R);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::R);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::R);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::R);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::WS);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::R);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::R);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::R);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::R);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::R);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::R);
    }
    {
        props_and_embeddings_t paes = {
            {'s', 1, bidi_prop_t::L, false},
            {'m', 1, bidi_prop_t::L, false},
            {'i', 1, bidi_prop_t::L, false},
            {'t', 1, bidi_prop_t::L, false},
            {'h', 1, bidi_prop_t::L, false},
            {' ', 1, bidi_prop_t::WS, false},
            {'(', 1, bidi_prop_t::ON, false},
            {'A', 1, bidi_prop_t::R, false},
            {'R', 1, bidi_prop_t::R, false},
            {'A', 1, bidi_prop_t::R, false},
            {'B', 1, bidi_prop_t::R, false},
            {'I', 1, bidi_prop_t::R, false},
            {'C', 1, bidi_prop_t::R, false},
            {' ', 1, bidi_prop_t::WS, false},
            {'f', 1, bidi_prop_t::L, false},
            {'a', 1, bidi_prop_t::L, false},
            {'b', 1, bidi_prop_t::L, false},
            {'r', 1, bidi_prop_t::L, false},
            {'i', 1, bidi_prop_t::L, false},
            {'k', 1, bidi_prop_t::L, false},
            {'a', 1, bidi_prop_t::L, false},
            {'m', 1, bidi_prop_t::L, false},
            {')', 1, bidi_prop_t::ON, false},
            {' ', 1, bidi_prop_t::WS, false},
            {'H', 1, bidi_prop_t::R, false},
            {'E', 1, bidi_prop_t::R, false},
            {'B', 1, bidi_prop_t::R, false},
            {'R', 1, bidi_prop_t::R, false},
            {'E', 1, bidi_prop_t::R, false},
            {'W', 1, bidi_prop_t::R, false},
        };

        auto runs = find_all_runs(paes.begin(), paes.end());
        auto run_sequences = find_run_sequences(paes, runs);
        EXPECT_EQ(run_sequences.size(), 1u);

        auto const bracket_pairs = find_bracket_pairs(run_sequences[0]);
        n0(run_sequences[0], bracket_pairs);

        auto it = run_sequences[0].begin();
        EXPECT_EQ(
            std::distance(run_sequences[0].begin(), run_sequences[0].end()),
            30);
        EXPECT_EQ(it->prop_, bidi_prop_t::L);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::L);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::L);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::L);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::L);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::WS);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::R);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::R);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::R);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::R);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::R);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::R);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::R);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::WS);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::L);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::L);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::L);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::L);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::L);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::L);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::L);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::L);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::R);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::WS);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::R);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::R);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::R);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::R);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::R);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::R);
    }
    {
        props_and_embeddings_t paes = {
            {'A', 1, bidi_prop_t::R, false},
            {'R', 1, bidi_prop_t::R, false},
            {'A', 1, bidi_prop_t::R, false},
            {'B', 1, bidi_prop_t::R, false},
            {'I', 1, bidi_prop_t::R, false},
            {'C', 1, bidi_prop_t::R, false},
            {' ', 1, bidi_prop_t::WS, false},
            {'b', 1, bidi_prop_t::L, false},
            {'o', 1, bidi_prop_t::L, false},
            {'o', 1, bidi_prop_t::L, false},
            {'k', 1, bidi_prop_t::L, false},
            {'(', 1, bidi_prop_t::ON, false},
            {'s', 1, bidi_prop_t::L, false},
            {')', 1, bidi_prop_t::ON, false},
        };

        auto runs = find_all_runs(paes.begin(), paes.end());
        auto run_sequences = find_run_sequences(paes, runs);
        EXPECT_EQ(run_sequences.size(), 1u);

        auto const bracket_pairs = find_bracket_pairs(run_sequences[0]);
        n0(run_sequences[0], bracket_pairs);

        auto it = run_sequences[0].begin();
        EXPECT_EQ(
            std::distance(run_sequences[0].begin(), run_sequences[0].end()),
            14);
        EXPECT_EQ(it->prop_, bidi_prop_t::R);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::R);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::R);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::R);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::R);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::R);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::WS);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::L);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::L);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::L);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::L);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::L);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::L);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::L);
    }
}

TEST(detail_bidi, n1_)
{
    {
        props_and_embeddings_t paes = {
            {0, 0, bidi_prop_t::L, false},
            {0, 0, bidi_prop_t::B, false},
            {0, 0, bidi_prop_t::L, false},
        };

        auto runs = find_all_runs(paes.begin(), paes.end());
        auto run_sequences = find_run_sequences(paes, runs);
        EXPECT_EQ(run_sequences.size(), 1u);

        n1(run_sequences[0]);

        auto it = run_sequences[0].begin();
        EXPECT_EQ(
            std::distance(run_sequences[0].begin(), run_sequences[0].end()), 3);
        EXPECT_EQ(it->prop_, bidi_prop_t::L);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::L);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::L);
    }
    {
        props_and_embeddings_t paes = {
            {0, 0, bidi_prop_t::R, false},
            {0, 0, bidi_prop_t::B, false},
            {0, 0, bidi_prop_t::R, false},
        };

        auto runs = find_all_runs(paes.begin(), paes.end());
        auto run_sequences = find_run_sequences(paes, runs);
        EXPECT_EQ(run_sequences.size(), 1u);

        n1(run_sequences[0]);

        auto it = run_sequences[0].begin();
        EXPECT_EQ(
            std::distance(run_sequences[0].begin(), run_sequences[0].end()), 3);
        EXPECT_EQ(it->prop_, bidi_prop_t::R);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::R);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::R);
    }
    {
        props_and_embeddings_t paes = {
            {0, 0, bidi_prop_t::R, false},
            {0, 0, bidi_prop_t::B, false},
            {0, 0, bidi_prop_t::AN, false},
        };

        auto runs = find_all_runs(paes.begin(), paes.end());
        auto run_sequences = find_run_sequences(paes, runs);
        EXPECT_EQ(run_sequences.size(), 1u);

        n1(run_sequences[0]);

        auto it = run_sequences[0].begin();
        EXPECT_EQ(
            std::distance(run_sequences[0].begin(), run_sequences[0].end()), 3);
        EXPECT_EQ(it->prop_, bidi_prop_t::R);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::R);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::AN);
    }
    {
        props_and_embeddings_t paes = {
            {0, 0, bidi_prop_t::R, false},
            {0, 0, bidi_prop_t::B, false},
            {0, 0, bidi_prop_t::EN, false},
        };

        auto runs = find_all_runs(paes.begin(), paes.end());
        auto run_sequences = find_run_sequences(paes, runs);
        EXPECT_EQ(run_sequences.size(), 1u);

        n1(run_sequences[0]);

        auto it = run_sequences[0].begin();
        EXPECT_EQ(
            std::distance(run_sequences[0].begin(), run_sequences[0].end()), 3);
        EXPECT_EQ(it->prop_, bidi_prop_t::R);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::R);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::EN);
    }
    {
        props_and_embeddings_t paes = {
            {0, 0, bidi_prop_t::AN, false},
            {0, 0, bidi_prop_t::B, false},
            {0, 0, bidi_prop_t::R, false},
        };

        auto runs = find_all_runs(paes.begin(), paes.end());
        auto run_sequences = find_run_sequences(paes, runs);
        EXPECT_EQ(run_sequences.size(), 1u);

        n1(run_sequences[0]);

        auto it = run_sequences[0].begin();
        EXPECT_EQ(
            std::distance(run_sequences[0].begin(), run_sequences[0].end()), 3);
        EXPECT_EQ(it->prop_, bidi_prop_t::AN);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::R);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::R);
    }
    {
        props_and_embeddings_t paes = {
            {0, 0, bidi_prop_t::AN, false},
            {0, 0, bidi_prop_t::B, false},
            {0, 0, bidi_prop_t::AN, false},
        };

        auto runs = find_all_runs(paes.begin(), paes.end());
        auto run_sequences = find_run_sequences(paes, runs);
        EXPECT_EQ(run_sequences.size(), 1u);

        n1(run_sequences[0]);

        auto it = run_sequences[0].begin();
        EXPECT_EQ(
            std::distance(run_sequences[0].begin(), run_sequences[0].end()), 3);
        EXPECT_EQ(it->prop_, bidi_prop_t::AN);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::R);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::AN);
    }
    {
        props_and_embeddings_t paes = {
            {0, 0, bidi_prop_t::AN, false},
            {0, 0, bidi_prop_t::B, false},
            {0, 0, bidi_prop_t::EN, false},
        };

        auto runs = find_all_runs(paes.begin(), paes.end());
        auto run_sequences = find_run_sequences(paes, runs);
        EXPECT_EQ(run_sequences.size(), 1u);

        n1(run_sequences[0]);

        auto it = run_sequences[0].begin();
        EXPECT_EQ(
            std::distance(run_sequences[0].begin(), run_sequences[0].end()), 3);
        EXPECT_EQ(it->prop_, bidi_prop_t::AN);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::R);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::EN);
    }
    {
        props_and_embeddings_t paes = {
            {0, 0, bidi_prop_t::EN, false},
            {0, 0, bidi_prop_t::B, false},
            {0, 0, bidi_prop_t::R, false},
        };

        auto runs = find_all_runs(paes.begin(), paes.end());
        auto run_sequences = find_run_sequences(paes, runs);
        EXPECT_EQ(run_sequences.size(), 1u);

        n1(run_sequences[0]);

        auto it = run_sequences[0].begin();
        EXPECT_EQ(
            std::distance(run_sequences[0].begin(), run_sequences[0].end()), 3);
        EXPECT_EQ(it->prop_, bidi_prop_t::EN);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::R);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::R);
    }
    {
        props_and_embeddings_t paes = {
            {0, 0, bidi_prop_t::EN, false},
            {0, 0, bidi_prop_t::B, false},
            {0, 0, bidi_prop_t::AN, false},
        };

        auto runs = find_all_runs(paes.begin(), paes.end());
        auto run_sequences = find_run_sequences(paes, runs);
        EXPECT_EQ(run_sequences.size(), 1u);

        n1(run_sequences[0]);

        auto it = run_sequences[0].begin();
        EXPECT_EQ(
            std::distance(run_sequences[0].begin(), run_sequences[0].end()), 3);
        EXPECT_EQ(it->prop_, bidi_prop_t::EN);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::R);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::AN);
    }
    {
        props_and_embeddings_t paes = {
            {0, 0, bidi_prop_t::EN, false},
            {0, 0, bidi_prop_t::B, false},
            {0, 0, bidi_prop_t::EN, false},
        };

        auto runs = find_all_runs(paes.begin(), paes.end());
        auto run_sequences = find_run_sequences(paes, runs);
        EXPECT_EQ(run_sequences.size(), 1u);

        n1(run_sequences[0]);

        auto it = run_sequences[0].begin();
        EXPECT_EQ(
            std::distance(run_sequences[0].begin(), run_sequences[0].end()), 3);
        EXPECT_EQ(it->prop_, bidi_prop_t::EN);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::R);
        ++it;
        EXPECT_EQ(it->prop_, bidi_prop_t::EN);
    }
}

TEST(detail_bidi, l2_)
{
    {
        props_and_embeddings_t paes = {
            {'c', 0, bidi_prop_t::L, false},
            {'a', 0, bidi_prop_t::L, false},
            {'r', 0, bidi_prop_t::L, false},
            {' ', 0, bidi_prop_t::WS, false},
            {'m', 0, bidi_prop_t::L, false},
            {'e', 0, bidi_prop_t::L, false},
            {'a', 0, bidi_prop_t::L, false},
            {'n', 0, bidi_prop_t::L, false},
            {'s', 0, bidi_prop_t::L, false},
            {' ', 0, bidi_prop_t::WS, false},
            {'C', 1, bidi_prop_t::R, false},
            {'A', 1, bidi_prop_t::R, false},
            {'R', 1, bidi_prop_t::R, false},
            {'.', 0, bidi_prop_t::CS, false},
        };

        auto runs = find_all_runs(paes.begin(), paes.end());
        reordered_runs_t reordered_runs = l2(runs);

        string result;
        for (auto run : reordered_runs) {
            for (auto pae : run) {
                result += (char)pae.cp_;
            }
        }

        EXPECT_EQ(result, "car means RAC.");
    }
    {
        props_and_embeddings_t paes = {
            {'<', 0, bidi_prop_t::RLI, false},
            {'c', 2, bidi_prop_t::L, false},
            {'a', 2, bidi_prop_t::L, false},
            {'r', 2, bidi_prop_t::L, false},
            {' ', 1, bidi_prop_t::WS, false},
            {'M', 1, bidi_prop_t::R, false},
            {'E', 1, bidi_prop_t::R, false},
            {'A', 1, bidi_prop_t::R, false},
            {'N', 1, bidi_prop_t::R, false},
            {'S', 1, bidi_prop_t::R, false},
            {' ', 1, bidi_prop_t::WS, false},
            {'C', 1, bidi_prop_t::R, false},
            {'A', 1, bidi_prop_t::R, false},
            {'R', 1, bidi_prop_t::R, false},
            {'.', 1, bidi_prop_t::CS, false},
            {'=', 0, bidi_prop_t::PDI, false},
        };

        auto runs = find_all_runs(paes.begin(), paes.end());
        reordered_runs_t reordered_runs = l2(runs);

        string result;
        for (auto run : reordered_runs) {
            for (auto pae : run) {
                result += (char)pae.cp_;
            }
        }

        EXPECT_EQ(result, "<.RAC SNAEM car=");
    }
    {
        props_and_embeddings_t paes = {
            {'h', 0, bidi_prop_t::L, false},
            {'e', 0, bidi_prop_t::L, false},
            {' ', 0, bidi_prop_t::WS, false},
            {'s', 0, bidi_prop_t::L, false},
            {'a', 0, bidi_prop_t::L, false},
            {'i', 0, bidi_prop_t::L, false},
            {'d', 0, bidi_prop_t::L, false},
            {' ', 0, bidi_prop_t::WS, false},
            {'"', 0, bidi_prop_t::ON, false},
            {'<', 0, bidi_prop_t::RLI, false},
            {'c', 2, bidi_prop_t::L, false},
            {'a', 2, bidi_prop_t::L, false},
            {'r', 2, bidi_prop_t::L, false},
            {' ', 1, bidi_prop_t::WS, false},
            {'M', 1, bidi_prop_t::R, false},
            {'E', 1, bidi_prop_t::R, false},
            {'A', 1, bidi_prop_t::R, false},
            {'N', 1, bidi_prop_t::R, false},
            {'S', 1, bidi_prop_t::R, false},
            {' ', 1, bidi_prop_t::WS, false},
            {'C', 1, bidi_prop_t::R, false},
            {'A', 1, bidi_prop_t::R, false},
            {'R', 1, bidi_prop_t::R, false},
            {'=', 0, bidi_prop_t::PDI, false},
            {'.', 0, bidi_prop_t::CS, false},
            {'"', 0, bidi_prop_t::ON, false},
            {' ', 0, bidi_prop_t::WS, false},
            {'"', 0, bidi_prop_t::ON, false},
            {'<', 0, bidi_prop_t::RLI, false},
            {'I', 1, bidi_prop_t::R, false},
            {'T', 1, bidi_prop_t::R, false},
            {' ', 1, bidi_prop_t::WS, false},
            {'D', 1, bidi_prop_t::R, false},
            {'O', 1, bidi_prop_t::R, false},
            {'E', 1, bidi_prop_t::R, false},
            {'S', 1, bidi_prop_t::R, false},
            {'=', 0, bidi_prop_t::PDI, false},
            {',', 0, bidi_prop_t::CS, false},
            {'"', 0, bidi_prop_t::ON, false},
            {' ', 0, bidi_prop_t::WS, false},
            {'s', 0, bidi_prop_t::L, false},
            {'h', 0, bidi_prop_t::L, false},
            {'e', 0, bidi_prop_t::L, false},
            {' ', 0, bidi_prop_t::WS, false},
            {'a', 0, bidi_prop_t::L, false},
            {'g', 0, bidi_prop_t::L, false},
            {'r', 0, bidi_prop_t::L, false},
            {'e', 0, bidi_prop_t::L, false},
            {'e', 0, bidi_prop_t::L, false},
            {'d', 0, bidi_prop_t::L, false},
            {'.', 0, bidi_prop_t::CS, false},
        };

        auto runs = find_all_runs(paes.begin(), paes.end());
        reordered_runs_t reordered_runs = l2(runs);

        string result;
        for (auto run : reordered_runs) {
            for (auto pae : run) {
                result += (char)pae.cp_;
            }
        }

        EXPECT_EQ(result, R"(he said “<RAC SNAEM car=.” “<SEOD TI=,” she agreed.)");
    }
    {
        props_and_embeddings_t paes = {
            {'D', 1, bidi_prop_t::R, false},
            {'I', 1, bidi_prop_t::R, false},
            {'D', 1, bidi_prop_t::R, false},
            {' ', 1, bidi_prop_t::WS, false},
            {'Y', 1, bidi_prop_t::R, false},
            {'O', 1, bidi_prop_t::R, false},
            {'U', 1, bidi_prop_t::R, false},
            {' ', 1, bidi_prop_t::WS, false},
            {'S', 1, bidi_prop_t::R, false},
            {'A', 1, bidi_prop_t::R, false},
            {'Y', 1, bidi_prop_t::R, false},
            {' ', 1, bidi_prop_t::WS, false},
            {'\'', 1, bidi_prop_t::ON, false},
            {'>', 1, bidi_prop_t::LRI, false},
            {'h', 2, bidi_prop_t::L, false},
            {'e', 2, bidi_prop_t::L, false},
            {' ', 2, bidi_prop_t::WS, false},
            {'s', 2, bidi_prop_t::L, false},
            {'a', 2, bidi_prop_t::L, false},
            {'i', 2, bidi_prop_t::L, false},
            {'d', 2, bidi_prop_t::L, false},
            {' ', 2, bidi_prop_t::WS, false},
            {'"', 2, bidi_prop_t::ON, false},
            {'<', 2, bidi_prop_t::RLI, false},
            {'c', 4, bidi_prop_t::L, false},
            {'a', 4, bidi_prop_t::L, false},
            {'r', 4, bidi_prop_t::L, false},
            {' ', 3, bidi_prop_t::WS, false},
            {'M', 3, bidi_prop_t::R, false},
            {'E', 3, bidi_prop_t::R, false},
            {'A', 3, bidi_prop_t::R, false},
            {'N', 3, bidi_prop_t::R, false},
            {'S', 3, bidi_prop_t::R, false},
            {' ', 3, bidi_prop_t::WS, false},
            {'C', 3, bidi_prop_t::R, false},
            {'A', 3, bidi_prop_t::R, false},
            {'R', 3, bidi_prop_t::R, false},
            {'=', 2, bidi_prop_t::PDI, false},
            {'"', 2, bidi_prop_t::ON, false},
            {'=', 1, bidi_prop_t::PDI, false},
            {'\'', 1, bidi_prop_t::ON, false},
            {'?', 1, bidi_prop_t::ON, false},
        };

        auto runs = find_all_runs(paes.begin(), paes.end());
        reordered_runs_t reordered_runs = l2(runs);

        string result;
        for (auto run : reordered_runs) {
            for (auto pae : run) {
                result += (char)pae.cp_;
            }
        }

        EXPECT_EQ(result, R"(?‘=he said “<RAC SNAEM car=”>’ YAS UOY DID)");
    }
}

// TODO: This is just here for early-stage testing.  Remove once an actual
// test exists for this function.
TEST(detail_bidi, bidirectional_order_instantiation)
{
    std::array<uint32_t, 1> str = {{'a'}};
    std::array<bidirectional_subrange<std::array<uint32_t, 1>::iterator>, 1024>
        subranges;
    bidirectional_order(str.begin(), str.end(), subranges.begin());
}
