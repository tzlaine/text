#include <boost/text/bidirectional.hpp>

#include <gtest/gtest.h>


using namespace boost::text;
using namespace boost::text::detail;


// https://unicode.org/reports/tr9/#BD13
TEST(detail_bidi, find_run_sequences_)
{
    auto run_used = [](level_run r) { return r.used_; };

    // Using bidi_property::L for all portions of the examples called "text".

    {
        props_and_embeddings_t paes = {
            // text1
            {0, 0, bidi_property::L, false},
            {0, 0, bidi_property::L, false},
            {0, 0, bidi_property::L, false},

            {0, 0, bidi_property::RLE, false},

            // text2
            {0, 1, bidi_property::L, false},
            {0, 1, bidi_property::L, false},
            {0, 1, bidi_property::L, false},

            {0, 1, bidi_property::PDF, false},

            {0, 1, bidi_property::RLE, false},

            // text3
            {0, 1, bidi_property::L, false},
            {0, 1, bidi_property::L, false},
            {0, 1, bidi_property::L, false},

            {0, 1, bidi_property::PDF, false},

            // text4
            {0, 0, bidi_property::L, false},
            {0, 0, bidi_property::L, false},
            {0, 0, bidi_property::L, false},
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
            {0, 0, bidi_property::L, false},
            {0, 0, bidi_property::L, false},
            {0, 0, bidi_property::L, false},

            {0, 0, bidi_property::RLI, false},

            // text2
            {0, 1, bidi_property::L, false},
            {0, 1, bidi_property::L, false},
            {0, 1, bidi_property::L, false},

            {0, 0, bidi_property::PDI, false},

            {0, 0, bidi_property::RLI, false},

            // text3
            {0, 1, bidi_property::L, false},
            {0, 1, bidi_property::L, false},
            {0, 1, bidi_property::L, false},

            {0, 0, bidi_property::PDI, false},

            // text4
            {0, 0, bidi_property::L, false},
            {0, 0, bidi_property::L, false},
            {0, 0, bidi_property::L, false},
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
            {0, 0, bidi_property::L, false},
            {0, 0, bidi_property::L, false},
            {0, 0, bidi_property::L, false},

            {0, 0, bidi_property::RLI, false},

            // text2
            {0, 1, bidi_property::L, false},
            {0, 1, bidi_property::L, false},
            {0, 1, bidi_property::L, false},

            {0, 1, bidi_property::LRI, false},

            // text3
            {0, 2, bidi_property::L, false},
            {0, 2, bidi_property::L, false},
            {0, 2, bidi_property::L, false},

            {0, 2, bidi_property::RLE, false},

            // text4
            {0, 3, bidi_property::L, false},
            {0, 3, bidi_property::L, false},
            {0, 3, bidi_property::L, false},

            {0, 3, bidi_property::PDF, false},

            // text5
            {0, 2, bidi_property::L, false},
            {0, 2, bidi_property::L, false},
            {0, 2, bidi_property::L, false},

            {0, 1, bidi_property::PDI, false},

            // text6
            {0, 1, bidi_property::L, false},
            {0, 1, bidi_property::L, false},
            {0, 1, bidi_property::L, false},

            {0, 0, bidi_property::PDI, false},

            // text7
            {0, 0, bidi_property::L, false},
            {0, 0, bidi_property::L, false},
            {0, 0, bidi_property::L, false},
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
            {0, 0, bidi_property::AL, false},
            {0, 0, bidi_property::NSM, false},
            {0, 0, bidi_property::NSM, false},
        };

        auto runs = find_all_runs(paes.begin(), paes.end());
        auto run_sequences = find_run_sequences(paes, runs);
        EXPECT_EQ(run_sequences.size(), 1u);

        find_sos_eos(run_sequences, 1); // 1 implies sos==R
        w1(run_sequences[0]);

        auto it = run_sequences[0].begin();
        EXPECT_EQ(
            std::distance(run_sequences[0].begin(), run_sequences[0].end()), 3);
        EXPECT_EQ(it->prop_, bidi_property::AL);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::AL);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::AL);
    }
    {
        props_and_embeddings_t paes = {
            {0, 0, bidi_property::NSM, false},
        };

        auto runs = find_all_runs(paes.begin(), paes.end());
        auto run_sequences = find_run_sequences(paes, runs);
        EXPECT_EQ(run_sequences.size(), 1u);

        find_sos_eos(run_sequences, 1); // 1 implies sos==R
        w1(run_sequences[0]);

        auto it = run_sequences[0].begin();
        EXPECT_EQ(
            std::distance(run_sequences[0].begin(), run_sequences[0].end()), 1);
        EXPECT_EQ(it->prop_, bidi_property::R);
    }
    {
        props_and_embeddings_t paes = {
            {0, 0, bidi_property::LRI, false},
            {0, 0, bidi_property::NSM, false},
        };

        auto runs = find_all_runs(paes.begin(), paes.end());
        auto run_sequences = find_run_sequences(paes, runs);
        EXPECT_EQ(run_sequences.size(), 1u);

        find_sos_eos(run_sequences, 1); // 1 implies sos==R
        w1(run_sequences[0]);

        auto it = run_sequences[0].begin();
        EXPECT_EQ(
            std::distance(run_sequences[0].begin(), run_sequences[0].end()), 2);
        EXPECT_EQ(it->prop_, bidi_property::LRI);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::ON);
    }
    {
        props_and_embeddings_t paes = {
            {0, 0, bidi_property::PDI, true},
            {0, 0, bidi_property::NSM, false},
        };

        auto runs = find_all_runs(paes.begin(), paes.end());
        auto run_sequences = find_run_sequences(paes, runs);
        EXPECT_EQ(run_sequences.size(), 1u);

        find_sos_eos(run_sequences, 1); // 1 implies sos==R
        w1(run_sequences[0]);

        auto it = run_sequences[0].begin();
        EXPECT_EQ(
            std::distance(run_sequences[0].begin(), run_sequences[0].end()), 2);
        EXPECT_EQ(it->prop_, bidi_property::PDI);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::ON);
    }
    {
        props_and_embeddings_t paes = {
            {0, 0, bidi_property::AL, false},
            {0, 0, bidi_property::BN, false},
            {0, 0, bidi_property::NSM, false},
            {0, 0, bidi_property::BN, false},
            {0, 0, bidi_property::NSM, false},
        };

        auto runs = find_all_runs(paes.begin(), paes.end());
        auto run_sequences = find_run_sequences(paes, runs);
        EXPECT_EQ(run_sequences.size(), 1u);

        find_sos_eos(run_sequences, 1); // 1 implies sos==R
        w1(run_sequences[0]);

        auto it = run_sequences[0].begin();
        EXPECT_EQ(
            std::distance(run_sequences[0].begin(), run_sequences[0].end()), 5);
        EXPECT_EQ(it->prop_, bidi_property::AL);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::BN);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::AL);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::BN);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::AL);
    }
    {
        props_and_embeddings_t paes = {
            {0, 0, bidi_property::BN, false},
            {0, 0, bidi_property::NSM, false},
        };

        auto runs = find_all_runs(paes.begin(), paes.end());
        auto run_sequences = find_run_sequences(paes, runs);
        EXPECT_EQ(run_sequences.size(), 1u);

        find_sos_eos(run_sequences, 1); // 1 implies sos==R
        w1(run_sequences[0]);

        auto it = run_sequences[0].begin();
        EXPECT_EQ(
            std::distance(run_sequences[0].begin(), run_sequences[0].end()), 2);
        EXPECT_EQ(it->prop_, bidi_property::BN);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::R);
    }

    // W2
    {
        props_and_embeddings_t paes = {
            {0, 0, bidi_property::AL, false},
            {0, 0, bidi_property::EN, false},
        };

        auto runs = find_all_runs(paes.begin(), paes.end());
        auto run_sequences = find_run_sequences(paes, runs);
        EXPECT_EQ(run_sequences.size(), 1u);

        find_sos_eos(run_sequences, 1); // 1 implies sos==R
        w2(run_sequences[0]);

        auto it = run_sequences[0].begin();
        EXPECT_EQ(
            std::distance(run_sequences[0].begin(), run_sequences[0].end()), 2);
        EXPECT_EQ(it->prop_, bidi_property::AL);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::AN);
    }
    {
        props_and_embeddings_t paes = {
            {0, 0, bidi_property::AL, false},
            {0, 0, bidi_property::B, false},
            {0, 0, bidi_property::EN, false},
        };

        auto runs = find_all_runs(paes.begin(), paes.end());
        auto run_sequences = find_run_sequences(paes, runs);
        EXPECT_EQ(run_sequences.size(), 1u);

        find_sos_eos(run_sequences, 1); // 1 implies sos==R
        w2(run_sequences[0]);

        auto it = run_sequences[0].begin();
        EXPECT_EQ(
            std::distance(run_sequences[0].begin(), run_sequences[0].end()), 3);
        EXPECT_EQ(it->prop_, bidi_property::AL);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::B);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::AN);
    }
    {
        props_and_embeddings_t paes = {
            {0, 0, bidi_property::B, false},
            {0, 0, bidi_property::EN, false},
        };

        auto runs = find_all_runs(paes.begin(), paes.end());
        auto run_sequences = find_run_sequences(paes, runs);
        EXPECT_EQ(run_sequences.size(), 1u);

        find_sos_eos(run_sequences, 1); // 1 implies sos==R
        w2(run_sequences[0]);

        auto it = run_sequences[0].begin();
        EXPECT_EQ(
            std::distance(run_sequences[0].begin(), run_sequences[0].end()), 2);
        EXPECT_EQ(it->prop_, bidi_property::B);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::EN);
    }
    {
        props_and_embeddings_t paes = {
            {0, 0, bidi_property::L, false},
            {0, 0, bidi_property::B, false},
            {0, 0, bidi_property::EN, false},
        };

        auto runs = find_all_runs(paes.begin(), paes.end());
        auto run_sequences = find_run_sequences(paes, runs);
        EXPECT_EQ(run_sequences.size(), 1u);

        find_sos_eos(run_sequences, 1); // 1 implies sos==R
        w2(run_sequences[0]);

        auto it = run_sequences[0].begin();
        EXPECT_EQ(
            std::distance(run_sequences[0].begin(), run_sequences[0].end()), 3);
        EXPECT_EQ(it->prop_, bidi_property::L);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::B);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::EN);
    }
    {
        props_and_embeddings_t paes = {
            {0, 0, bidi_property::R, false},
            {0, 0, bidi_property::B, false},
            {0, 0, bidi_property::EN, false},
        };

        auto runs = find_all_runs(paes.begin(), paes.end());
        auto run_sequences = find_run_sequences(paes, runs);
        EXPECT_EQ(run_sequences.size(), 1u);

        find_sos_eos(run_sequences, 1); // 1 implies sos==R
        w2(run_sequences[0]);

        auto it = run_sequences[0].begin();
        EXPECT_EQ(
            std::distance(run_sequences[0].begin(), run_sequences[0].end()), 3);
        EXPECT_EQ(it->prop_, bidi_property::R);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::B);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::EN);
    }

    // W4
    {
        props_and_embeddings_t paes = {
            {0, 0, bidi_property::EN, false},
            {0, 0, bidi_property::ES, false},
            {0, 0, bidi_property::EN, false},
        };

        auto runs = find_all_runs(paes.begin(), paes.end());
        auto run_sequences = find_run_sequences(paes, runs);
        EXPECT_EQ(run_sequences.size(), 1u);

        find_sos_eos(run_sequences, 1); // 1 implies sos==R
        w4(run_sequences[0]);

        auto it = run_sequences[0].begin();
        EXPECT_EQ(
            std::distance(run_sequences[0].begin(), run_sequences[0].end()), 3);
        EXPECT_EQ(it->prop_, bidi_property::EN);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::EN);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::EN);
    }
    {
        props_and_embeddings_t paes = {
            {0, 0, bidi_property::EN, false},
            {0, 0, bidi_property::CS, false},
            {0, 0, bidi_property::EN, false},
        };

        auto runs = find_all_runs(paes.begin(), paes.end());
        auto run_sequences = find_run_sequences(paes, runs);
        EXPECT_EQ(run_sequences.size(), 1u);

        find_sos_eos(run_sequences, 1); // 1 implies sos==R
        w4(run_sequences[0]);

        auto it = run_sequences[0].begin();
        EXPECT_EQ(
            std::distance(run_sequences[0].begin(), run_sequences[0].end()), 3);
        EXPECT_EQ(it->prop_, bidi_property::EN);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::EN);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::EN);
    }
    {
        props_and_embeddings_t paes = {
            {0, 0, bidi_property::AN, false},
            {0, 0, bidi_property::CS, false},
            {0, 0, bidi_property::AN, false},
        };

        auto runs = find_all_runs(paes.begin(), paes.end());
        auto run_sequences = find_run_sequences(paes, runs);
        EXPECT_EQ(run_sequences.size(), 1u);

        find_sos_eos(run_sequences, 1); // 1 implies sos==R
        w4(run_sequences[0]);

        auto it = run_sequences[0].begin();
        EXPECT_EQ(
            std::distance(run_sequences[0].begin(), run_sequences[0].end()), 3);
        EXPECT_EQ(it->prop_, bidi_property::AN);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::AN);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::AN);
    }
    {
        props_and_embeddings_t paes = {
            {0, 0, bidi_property::BN, false},
            {0, 0, bidi_property::EN, false},
            {0, 0, bidi_property::BN, false},
            {0, 0, bidi_property::BN, false},
            {0, 0, bidi_property::ES, false},
            {0, 0, bidi_property::BN, false},
            {0, 0, bidi_property::EN, false},
            {0, 0, bidi_property::BN, false},
        };

        auto runs = find_all_runs(paes.begin(), paes.end());
        auto run_sequences = find_run_sequences(paes, runs);
        EXPECT_EQ(run_sequences.size(), 1u);

        find_sos_eos(run_sequences, 1); // 1 implies sos==R
        w4(run_sequences[0]);

        auto it = run_sequences[0].begin();
        EXPECT_EQ(
            std::distance(run_sequences[0].begin(), run_sequences[0].end()), 8);
        EXPECT_EQ(it->prop_, bidi_property::BN);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::EN);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::BN);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::BN);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::EN);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::BN);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::EN);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::BN);
    }
    {
        props_and_embeddings_t paes = {
            {0, 0, bidi_property::BN, false},
            {0, 0, bidi_property::EN, false},
            {0, 0, bidi_property::BN, false},
            {0, 0, bidi_property::CS, false},
            {0, 0, bidi_property::BN, false},
            {0, 0, bidi_property::EN, false},
            {0, 0, bidi_property::BN, false},
        };

        auto runs = find_all_runs(paes.begin(), paes.end());
        auto run_sequences = find_run_sequences(paes, runs);
        EXPECT_EQ(run_sequences.size(), 1u);

        find_sos_eos(run_sequences, 1); // 1 implies sos==R
        w4(run_sequences[0]);

        auto it = run_sequences[0].begin();
        EXPECT_EQ(
            std::distance(run_sequences[0].begin(), run_sequences[0].end()), 7);
        EXPECT_EQ(it->prop_, bidi_property::BN);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::EN);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::BN);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::EN);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::BN);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::EN);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::BN);
    }
    {
        props_and_embeddings_t paes = {
            {0, 0, bidi_property::BN, false},
            {0, 0, bidi_property::BN, false},
            {0, 0, bidi_property::AN, false},
            {0, 0, bidi_property::BN, false},
            {0, 0, bidi_property::CS, false},
            {0, 0, bidi_property::BN, false},
            {0, 0, bidi_property::AN, false},
            {0, 0, bidi_property::BN, false},
        };

        auto runs = find_all_runs(paes.begin(), paes.end());
        auto run_sequences = find_run_sequences(paes, runs);
        EXPECT_EQ(run_sequences.size(), 1u);

        find_sos_eos(run_sequences, 1); // 1 implies sos==R
        w4(run_sequences[0]);

        auto it = run_sequences[0].begin();
        EXPECT_EQ(
            std::distance(run_sequences[0].begin(), run_sequences[0].end()), 8);
        EXPECT_EQ(it->prop_, bidi_property::BN);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::BN);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::AN);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::BN);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::AN);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::BN);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::AN);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::BN);
    }

    // W5
    {
        props_and_embeddings_t paes = {
            {0, 0, bidi_property::ET, false},
            {0, 0, bidi_property::ET, false},
            {0, 0, bidi_property::EN, false},
        };

        auto runs = find_all_runs(paes.begin(), paes.end());
        auto run_sequences = find_run_sequences(paes, runs);
        EXPECT_EQ(run_sequences.size(), 1u);

        find_sos_eos(run_sequences, 1); // 1 implies sos==R
        w5(run_sequences[0]);

        auto it = run_sequences[0].begin();
        EXPECT_EQ(
            std::distance(run_sequences[0].begin(), run_sequences[0].end()), 3);
        EXPECT_EQ(it->prop_, bidi_property::EN);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::EN);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::EN);
    }
    {
        props_and_embeddings_t paes = {
            {0, 0, bidi_property::EN, false},
            {0, 0, bidi_property::ET, false},
            {0, 0, bidi_property::ET, false},
        };

        auto runs = find_all_runs(paes.begin(), paes.end());
        auto run_sequences = find_run_sequences(paes, runs);
        EXPECT_EQ(run_sequences.size(), 1u);

        find_sos_eos(run_sequences, 1); // 1 implies sos==R
        w5(run_sequences[0]);

        auto it = run_sequences[0].begin();
        EXPECT_EQ(
            std::distance(run_sequences[0].begin(), run_sequences[0].end()), 3);
        EXPECT_EQ(it->prop_, bidi_property::EN);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::EN);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::EN);
    }
    {
        props_and_embeddings_t paes = {
            {0, 0, bidi_property::AN, false},
            {0, 0, bidi_property::ET, false},
            {0, 0, bidi_property::EN, false},
        };

        auto runs = find_all_runs(paes.begin(), paes.end());
        auto run_sequences = find_run_sequences(paes, runs);
        EXPECT_EQ(run_sequences.size(), 1u);

        find_sos_eos(run_sequences, 1); // 1 implies sos==R
        w5(run_sequences[0]);

        auto it = run_sequences[0].begin();
        EXPECT_EQ(
            std::distance(run_sequences[0].begin(), run_sequences[0].end()), 3);
        EXPECT_EQ(it->prop_, bidi_property::AN);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::EN);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::EN);
    }
    {
        props_and_embeddings_t paes = {
            {0, 0, bidi_property::ET, false},
            {0, 0, bidi_property::AN, false},
            {0, 0, bidi_property::EN, false},
        };

        auto runs = find_all_runs(paes.begin(), paes.end());
        auto run_sequences = find_run_sequences(paes, runs);
        EXPECT_EQ(run_sequences.size(), 1u);

        find_sos_eos(run_sequences, 1); // 1 implies sos==R
        w5(run_sequences[0]);

        auto it = run_sequences[0].begin();
        EXPECT_EQ(
            std::distance(run_sequences[0].begin(), run_sequences[0].end()), 3);
        EXPECT_EQ(it->prop_, bidi_property::ET);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::AN);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::EN);
    }
    {
        props_and_embeddings_t paes = {
            {0, 0, bidi_property::EN, false},
            {0, 0, bidi_property::AN, false},
            {0, 0, bidi_property::ET, false},
        };

        auto runs = find_all_runs(paes.begin(), paes.end());
        auto run_sequences = find_run_sequences(paes, runs);
        EXPECT_EQ(run_sequences.size(), 1u);

        find_sos_eos(run_sequences, 1); // 1 implies sos==R
        w5(run_sequences[0]);

        auto it = run_sequences[0].begin();
        EXPECT_EQ(
            std::distance(run_sequences[0].begin(), run_sequences[0].end()), 3);
        EXPECT_EQ(it->prop_, bidi_property::EN);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::AN);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::ET);
    }
    {
        props_and_embeddings_t paes = {
            {0, 0, bidi_property::EN, false},
            {0, 0, bidi_property::EN, false},
            {0, 0, bidi_property::ET, false},
        };

        auto runs = find_all_runs(paes.begin(), paes.end());
        auto run_sequences = find_run_sequences(paes, runs);
        EXPECT_EQ(run_sequences.size(), 1u);

        find_sos_eos(run_sequences, 1); // 1 implies sos==R
        w5(run_sequences[0]);

        auto it = run_sequences[0].begin();
        EXPECT_EQ(
            std::distance(run_sequences[0].begin(), run_sequences[0].end()), 3);
        EXPECT_EQ(it->prop_, bidi_property::EN);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::EN);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::EN);
    }
    {
        props_and_embeddings_t paes = {
            {0, 0, bidi_property::BN, false},
            {0, 0, bidi_property::ET, false},
            {0, 0, bidi_property::ET, false},
            {0, 0, bidi_property::BN, false},
            {0, 0, bidi_property::EN, false},
        };

        auto runs = find_all_runs(paes.begin(), paes.end());
        auto run_sequences = find_run_sequences(paes, runs);
        EXPECT_EQ(run_sequences.size(), 1u);

        find_sos_eos(run_sequences, 1); // 1 implies sos==R
        w5(run_sequences[0]);

        auto it = run_sequences[0].begin();
        EXPECT_EQ(
            std::distance(run_sequences[0].begin(), run_sequences[0].end()), 5);
        EXPECT_EQ(it->prop_, bidi_property::EN);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::EN);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::EN);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::EN);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::EN);
    }
    {
        props_and_embeddings_t paes = {
            {0, 0, bidi_property::BN, false},
            {0, 0, bidi_property::EN, false},
            {0, 0, bidi_property::BN, false},
            {0, 0, bidi_property::ET, false},
            {0, 0, bidi_property::ET, false},
            {0, 0, bidi_property::BN, false},
        };

        auto runs = find_all_runs(paes.begin(), paes.end());
        auto run_sequences = find_run_sequences(paes, runs);
        EXPECT_EQ(run_sequences.size(), 1u);

        find_sos_eos(run_sequences, 1); // 1 implies sos==R
        w5(run_sequences[0]);

        auto it = run_sequences[0].begin();
        EXPECT_EQ(
            std::distance(run_sequences[0].begin(), run_sequences[0].end()), 6);
        EXPECT_EQ(it->prop_, bidi_property::EN);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::EN);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::EN);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::EN);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::EN);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::EN);
    }

    // W6
    {
        props_and_embeddings_t paes = {
            {0, 0, bidi_property::AN, false},
            {0, 0, bidi_property::ET, false},
        };

        auto runs = find_all_runs(paes.begin(), paes.end());
        auto run_sequences = find_run_sequences(paes, runs);
        EXPECT_EQ(run_sequences.size(), 1u);

        find_sos_eos(run_sequences, 1); // 1 implies sos==R
        w6(run_sequences[0]);

        auto it = run_sequences[0].begin();
        EXPECT_EQ(
            std::distance(run_sequences[0].begin(), run_sequences[0].end()), 2);
        EXPECT_EQ(it->prop_, bidi_property::AN);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::ON);
    }
    {
        props_and_embeddings_t paes = {
            {0, 0, bidi_property::L, false},
            {0, 0, bidi_property::ES, false},
            {0, 0, bidi_property::EN, false},
        };

        auto runs = find_all_runs(paes.begin(), paes.end());
        auto run_sequences = find_run_sequences(paes, runs);
        EXPECT_EQ(run_sequences.size(), 1u);

        find_sos_eos(run_sequences, 1); // 1 implies sos==R
        w6(run_sequences[0]);

        auto it = run_sequences[0].begin();
        EXPECT_EQ(
            std::distance(run_sequences[0].begin(), run_sequences[0].end()), 3);
        EXPECT_EQ(it->prop_, bidi_property::L);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::ON);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::EN);
    }
    {
        props_and_embeddings_t paes = {
            {0, 0, bidi_property::EN, false},
            {0, 0, bidi_property::CS, false},
            {0, 0, bidi_property::AN, false},
        };

        auto runs = find_all_runs(paes.begin(), paes.end());
        auto run_sequences = find_run_sequences(paes, runs);
        EXPECT_EQ(run_sequences.size(), 1u);

        find_sos_eos(run_sequences, 1); // 1 implies sos==R
        w6(run_sequences[0]);

        auto it = run_sequences[0].begin();
        EXPECT_EQ(
            std::distance(run_sequences[0].begin(), run_sequences[0].end()), 3);
        EXPECT_EQ(it->prop_, bidi_property::EN);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::ON);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::AN);
    }
    {
        props_and_embeddings_t paes = {
            {0, 0, bidi_property::ET, false},
            {0, 0, bidi_property::AN, false},
        };

        auto runs = find_all_runs(paes.begin(), paes.end());
        auto run_sequences = find_run_sequences(paes, runs);
        EXPECT_EQ(run_sequences.size(), 1u);

        find_sos_eos(run_sequences, 1); // 1 implies sos==R
        w6(run_sequences[0]);

        auto it = run_sequences[0].begin();
        EXPECT_EQ(
            std::distance(run_sequences[0].begin(), run_sequences[0].end()), 2);
        EXPECT_EQ(it->prop_, bidi_property::ON);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::AN);
    }

    {
        props_and_embeddings_t paes = {
            {0, 0, bidi_property::BN, false},
            {0, 0, bidi_property::AN, false},
            {0, 0, bidi_property::BN, false},
            {0, 0, bidi_property::ET, false},
        };

        auto runs = find_all_runs(paes.begin(), paes.end());
        auto run_sequences = find_run_sequences(paes, runs);
        EXPECT_EQ(run_sequences.size(), 1u);

        find_sos_eos(run_sequences, 1); // 1 implies sos==R
        w6(run_sequences[0]);

        auto it = run_sequences[0].begin();
        EXPECT_EQ(
            std::distance(run_sequences[0].begin(), run_sequences[0].end()), 4);
        EXPECT_EQ(it->prop_, bidi_property::BN);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::AN);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::ON);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::ON);
    }
    {
        props_and_embeddings_t paes = {
            {0, 0, bidi_property::L, false},
            {0, 0, bidi_property::ES, false},
            {0, 0, bidi_property::BN, false},
            {0, 0, bidi_property::EN, false},
            {0, 0, bidi_property::BN, false},
        };

        auto runs = find_all_runs(paes.begin(), paes.end());
        auto run_sequences = find_run_sequences(paes, runs);
        EXPECT_EQ(run_sequences.size(), 1u);

        find_sos_eos(run_sequences, 1); // 1 implies sos==R
        w6(run_sequences[0]);

        auto it = run_sequences[0].begin();
        EXPECT_EQ(
            std::distance(run_sequences[0].begin(), run_sequences[0].end()), 5);
        EXPECT_EQ(it->prop_, bidi_property::L);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::ON);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::ON);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::EN);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::BN);
    }
    {
        props_and_embeddings_t paes = {
            {0, 0, bidi_property::EN, false},
            {0, 0, bidi_property::BN, false},
            {0, 0, bidi_property::CS, false},
            {0, 0, bidi_property::AN, false},
        };

        auto runs = find_all_runs(paes.begin(), paes.end());
        auto run_sequences = find_run_sequences(paes, runs);
        EXPECT_EQ(run_sequences.size(), 1u);

        find_sos_eos(run_sequences, 1); // 1 implies sos==R
        w6(run_sequences[0]);

        auto it = run_sequences[0].begin();
        EXPECT_EQ(
            std::distance(run_sequences[0].begin(), run_sequences[0].end()), 4);
        EXPECT_EQ(it->prop_, bidi_property::EN);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::ON);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::ON);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::AN);
    }
    {
        props_and_embeddings_t paes = {
            {0, 0, bidi_property::BN, false},
            {0, 0, bidi_property::ET, false},
            {0, 0, bidi_property::AN, false},
        };

        auto runs = find_all_runs(paes.begin(), paes.end());
        auto run_sequences = find_run_sequences(paes, runs);
        EXPECT_EQ(run_sequences.size(), 1u);

        find_sos_eos(run_sequences, 1); // 1 implies sos==R
        w6(run_sequences[0]);

        auto it = run_sequences[0].begin();
        EXPECT_EQ(
            std::distance(run_sequences[0].begin(), run_sequences[0].end()), 3);
        EXPECT_EQ(it->prop_, bidi_property::ON);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::ON);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::AN);
    }
    {
        props_and_embeddings_t paes = {
            {0, 0, bidi_property::ET, false},
            {0, 0, bidi_property::BN, false},
            {0, 0, bidi_property::AN, false},
            {0, 0, bidi_property::BN, false},
        };

        auto runs = find_all_runs(paes.begin(), paes.end());
        auto run_sequences = find_run_sequences(paes, runs);
        EXPECT_EQ(run_sequences.size(), 1u);

        find_sos_eos(run_sequences, 1); // 1 implies sos==R
        w6(run_sequences[0]);

        auto it = run_sequences[0].begin();
        EXPECT_EQ(
            std::distance(run_sequences[0].begin(), run_sequences[0].end()), 4);
        EXPECT_EQ(it->prop_, bidi_property::ON);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::ON);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::AN);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::BN);
    }

    // W7
    {
        props_and_embeddings_t paes = {
            {0, 0, bidi_property::L, false},
            {0, 0, bidi_property::B, false},
            {0, 0, bidi_property::EN, false},
        };

        auto runs = find_all_runs(paes.begin(), paes.end());
        auto run_sequences = find_run_sequences(paes, runs);
        EXPECT_EQ(run_sequences.size(), 1u);

        find_sos_eos(run_sequences, 1); // 1 implies sos==R
        w7(run_sequences[0]);

        auto it = run_sequences[0].begin();
        EXPECT_EQ(
            std::distance(run_sequences[0].begin(), run_sequences[0].end()), 3);
        EXPECT_EQ(it->prop_, bidi_property::L);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::B);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::L);
    }
    {
        props_and_embeddings_t paes = {
            {0, 0, bidi_property::R, false},
            {0, 0, bidi_property::B, false},
            {0, 0, bidi_property::EN, false},
        };

        auto runs = find_all_runs(paes.begin(), paes.end());
        auto run_sequences = find_run_sequences(paes, runs);
        EXPECT_EQ(run_sequences.size(), 1u);

        find_sos_eos(run_sequences, 1); // 1 implies sos==R
        w7(run_sequences[0]);

        auto it = run_sequences[0].begin();
        EXPECT_EQ(
            std::distance(run_sequences[0].begin(), run_sequences[0].end()), 3);
        EXPECT_EQ(it->prop_, bidi_property::R);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::B);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::EN);
    }
    {
        props_and_embeddings_t paes = {
            {0, 0, bidi_property::L, false},
            {0, 0, bidi_property::BN, false},
            {0, 0, bidi_property::B, false},
            {0, 0, bidi_property::EN, false},
        };

        auto runs = find_all_runs(paes.begin(), paes.end());
        auto run_sequences = find_run_sequences(paes, runs);
        EXPECT_EQ(run_sequences.size(), 1u);

        find_sos_eos(run_sequences, 1); // 1 implies sos==R
        w7(run_sequences[0]);

        auto it = run_sequences[0].begin();
        EXPECT_EQ(
            std::distance(run_sequences[0].begin(), run_sequences[0].end()), 4);
        EXPECT_EQ(it->prop_, bidi_property::L);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::BN);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::B);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::L);
    }
    {
        props_and_embeddings_t paes = {
            {0, 0, bidi_property::R, false},
            {0, 0, bidi_property::B, false},
            {0, 0, bidi_property::BN, false},
            {0, 0, bidi_property::EN, false},
        };

        auto runs = find_all_runs(paes.begin(), paes.end());
        auto run_sequences = find_run_sequences(paes, runs);
        EXPECT_EQ(run_sequences.size(), 1u);

        find_sos_eos(run_sequences, 1); // 1 implies sos==R
        w7(run_sequences[0]);

        auto it = run_sequences[0].begin();
        EXPECT_EQ(
            std::distance(run_sequences[0].begin(), run_sequences[0].end()), 4);
        EXPECT_EQ(it->prop_, bidi_property::R);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::B);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::BN);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::EN);
    }
}

TEST(detail_bidi, find_bracket_pairs_)
{
    {
        props_and_embeddings_t paes = {
            {'a', 0, bidi_property::ON, false},
            {')', 0, bidi_property::ON, false},
            {'b', 0, bidi_property::ON, false},
            {'(', 0, bidi_property::ON, false},
            {'c', 0, bidi_property::ON, false},
        };

        auto runs = find_all_runs(paes.begin(), paes.end());
        auto run_sequences = find_run_sequences(paes, runs);
        EXPECT_EQ(run_sequences.size(), 1u);

        auto const bracket_pairs = find_bracket_pairs(run_sequences[0]);

        EXPECT_EQ(std::distance(bracket_pairs.begin(), bracket_pairs.end()), 0);
    }
    {
        props_and_embeddings_t paes = {
            {'a', 0, bidi_property::ON, false},
            {')', 0, bidi_property::ON, false},
            {'b', 0, bidi_property::ON, false},
            {']', 0, bidi_property::ON, false},
            {'c', 0, bidi_property::ON, false},
        };

        auto runs = find_all_runs(paes.begin(), paes.end());
        auto run_sequences = find_run_sequences(paes, runs);
        EXPECT_EQ(run_sequences.size(), 1u);

        auto const bracket_pairs = find_bracket_pairs(run_sequences[0]);

        EXPECT_EQ(std::distance(bracket_pairs.begin(), bracket_pairs.end()), 0);
    }
    {
        props_and_embeddings_t paes = {
            {'a', 0, bidi_property::ON, false},
            {'(', 0, bidi_property::ON, false},
            {'b', 0, bidi_property::ON, false},
            {')', 0, bidi_property::ON, false},
            {'c', 0, bidi_property::ON, false},
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
            {'a', 0, bidi_property::ON, false},
            {'(', 0, bidi_property::ON, false},
            {'b', 0, bidi_property::ON, false},
            {'[', 0, bidi_property::ON, false},
            {'c', 0, bidi_property::ON, false},
            {')', 0, bidi_property::ON, false},
            {'d', 0, bidi_property::ON, false},
            {']', 0, bidi_property::ON, false},
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
            {'a', 0, bidi_property::ON, false},
            {'(', 0, bidi_property::ON, false},
            {'b', 0, bidi_property::ON, false},
            {']', 0, bidi_property::ON, false},
            {'c', 0, bidi_property::ON, false},
            {')', 0, bidi_property::ON, false},
            {'d', 0, bidi_property::ON, false},
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
            {'a', 0, bidi_property::ON, false},
            {'(', 0, bidi_property::ON, false},
            {'b', 0, bidi_property::ON, false},
            {')', 0, bidi_property::ON, false},
            {'c', 0, bidi_property::ON, false},
            {')', 0, bidi_property::ON, false},
            {'d', 0, bidi_property::ON, false},
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
            {'a', 0, bidi_property::ON, false},
            {'(', 0, bidi_property::ON, false},
            {'b', 0, bidi_property::ON, false},
            {'(', 0, bidi_property::ON, false},
            {'c', 0, bidi_property::ON, false},
            {')', 0, bidi_property::ON, false},
            {'d', 0, bidi_property::ON, false},
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
            {'a', 0, bidi_property::ON, false},
            {'(', 0, bidi_property::ON, false},
            {'b', 0, bidi_property::ON, false},
            {'(', 0, bidi_property::ON, false},
            {'c', 0, bidi_property::ON, false},
            {')', 0, bidi_property::ON, false},
            {'d', 0, bidi_property::ON, false},
            {')', 0, bidi_property::ON, false},
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
            {'a', 0, bidi_property::ON, false},
            {'(', 0, bidi_property::ON, false},
            {'b', 0, bidi_property::ON, false},
            {'{', 0, bidi_property::ON, false},
            {'c', 0, bidi_property::ON, false},
            {'}', 0, bidi_property::ON, false},
            {'d', 0, bidi_property::ON, false},
            {')', 0, bidi_property::ON, false},
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
            {'A', 1, bidi_property::R, false},
            {'B', 1, bidi_property::R, false},
            {'(', 1, bidi_property::ON, false},
            {'C', 1, bidi_property::R, false},
            {'D', 1, bidi_property::R, false},
            {'[', 1, bidi_property::ON, false},
            {'&', 1, bidi_property::ON, false},
            {'e', 1, bidi_property::L, false},
            {'f', 1, bidi_property::L, false},
            {']', 1, bidi_property::ON, false},
            {'!', 1, bidi_property::ON, false},
            {')', 1, bidi_property::ON, false},
            {'g', 1, bidi_property::L, false},
            {'h', 1, bidi_property::L, false},
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
        EXPECT_EQ(it->prop_, bidi_property::R);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::R);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::R);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::R);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::R);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::R);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::ON);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::L);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::L);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::R);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::ON);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::R);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::L);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::L);
    }
    {
        props_and_embeddings_t paes = {
            {'s', 1, bidi_property::L, false},
            {'m', 1, bidi_property::L, false},
            {'i', 1, bidi_property::L, false},
            {'t', 1, bidi_property::L, false},
            {'h', 1, bidi_property::L, false},
            {' ', 1, bidi_property::WS, false},
            {'(', 1, bidi_property::ON, false},
            {'f', 1, bidi_property::L, false},
            {'a', 1, bidi_property::L, false},
            {'b', 1, bidi_property::L, false},
            {'r', 1, bidi_property::L, false},
            {'i', 1, bidi_property::L, false},
            {'k', 1, bidi_property::L, false},
            {'a', 1, bidi_property::L, false},
            {'m', 1, bidi_property::L, false},
            {' ', 1, bidi_property::WS, false},
            {'A', 1, bidi_property::R, false},
            {'R', 1, bidi_property::R, false},
            {'A', 1, bidi_property::R, false},
            {'B', 1, bidi_property::R, false},
            {'I', 1, bidi_property::R, false},
            {'C', 1, bidi_property::R, false},
            {')', 1, bidi_property::ON, false},
            {' ', 1, bidi_property::WS, false},
            {'H', 1, bidi_property::R, false},
            {'E', 1, bidi_property::R, false},
            {'B', 1, bidi_property::R, false},
            {'R', 1, bidi_property::R, false},
            {'E', 1, bidi_property::R, false},
            {'W', 1, bidi_property::R, false},
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
        EXPECT_EQ(it->prop_, bidi_property::L);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::L);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::L);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::L);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::L);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::WS);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::R);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::L);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::L);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::L);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::L);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::L);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::L);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::L);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::L);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::WS);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::R);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::R);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::R);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::R);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::R);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::R);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::R);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::WS);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::R);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::R);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::R);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::R);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::R);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::R);
    }
    {
        props_and_embeddings_t paes = {
            {'s', 1, bidi_property::L, false},
            {'m', 1, bidi_property::L, false},
            {'i', 1, bidi_property::L, false},
            {'t', 1, bidi_property::L, false},
            {'h', 1, bidi_property::L, false},
            {' ', 1, bidi_property::WS, false},
            {'(', 1, bidi_property::ON, false},
            {'A', 1, bidi_property::R, false},
            {'R', 1, bidi_property::R, false},
            {'A', 1, bidi_property::R, false},
            {'B', 1, bidi_property::R, false},
            {'I', 1, bidi_property::R, false},
            {'C', 1, bidi_property::R, false},
            {' ', 1, bidi_property::WS, false},
            {'f', 1, bidi_property::L, false},
            {'a', 1, bidi_property::L, false},
            {'b', 1, bidi_property::L, false},
            {'r', 1, bidi_property::L, false},
            {'i', 1, bidi_property::L, false},
            {'k', 1, bidi_property::L, false},
            {'a', 1, bidi_property::L, false},
            {'m', 1, bidi_property::L, false},
            {')', 1, bidi_property::ON, false},
            {' ', 1, bidi_property::WS, false},
            {'H', 1, bidi_property::R, false},
            {'E', 1, bidi_property::R, false},
            {'B', 1, bidi_property::R, false},
            {'R', 1, bidi_property::R, false},
            {'E', 1, bidi_property::R, false},
            {'W', 1, bidi_property::R, false},
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
        EXPECT_EQ(it->prop_, bidi_property::L);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::L);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::L);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::L);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::L);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::WS);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::R);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::R);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::R);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::R);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::R);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::R);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::R);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::WS);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::L);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::L);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::L);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::L);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::L);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::L);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::L);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::L);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::R);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::WS);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::R);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::R);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::R);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::R);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::R);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::R);
    }
    {
        props_and_embeddings_t paes = {
            {'A', 1, bidi_property::R, false},
            {'R', 1, bidi_property::R, false},
            {'A', 1, bidi_property::R, false},
            {'B', 1, bidi_property::R, false},
            {'I', 1, bidi_property::R, false},
            {'C', 1, bidi_property::R, false},
            {' ', 1, bidi_property::WS, false},
            {'b', 1, bidi_property::L, false},
            {'o', 1, bidi_property::L, false},
            {'o', 1, bidi_property::L, false},
            {'k', 1, bidi_property::L, false},
            {'(', 1, bidi_property::ON, false},
            {'s', 1, bidi_property::L, false},
            {')', 1, bidi_property::ON, false},
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
        EXPECT_EQ(it->prop_, bidi_property::R);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::R);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::R);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::R);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::R);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::R);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::WS);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::L);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::L);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::L);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::L);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::L);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::L);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::L);
    }
}

TEST(detail_bidi, n1_)
{
    {
        props_and_embeddings_t paes = {
            {0, 0, bidi_property::L, false},
            {0, 0, bidi_property::B, false},
            {0, 0, bidi_property::L, false},
        };

        auto runs = find_all_runs(paes.begin(), paes.end());
        auto run_sequences = find_run_sequences(paes, runs);
        EXPECT_EQ(run_sequences.size(), 1u);

        n1(run_sequences[0]);

        auto it = run_sequences[0].begin();
        EXPECT_EQ(
            std::distance(run_sequences[0].begin(), run_sequences[0].end()), 3);
        EXPECT_EQ(it->prop_, bidi_property::L);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::L);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::L);
    }
    {
        props_and_embeddings_t paes = {
            {0, 0, bidi_property::R, false},
            {0, 0, bidi_property::B, false},
            {0, 0, bidi_property::R, false},
        };

        auto runs = find_all_runs(paes.begin(), paes.end());
        auto run_sequences = find_run_sequences(paes, runs);
        EXPECT_EQ(run_sequences.size(), 1u);

        n1(run_sequences[0]);

        auto it = run_sequences[0].begin();
        EXPECT_EQ(
            std::distance(run_sequences[0].begin(), run_sequences[0].end()), 3);
        EXPECT_EQ(it->prop_, bidi_property::R);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::R);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::R);
    }
    {
        props_and_embeddings_t paes = {
            {0, 0, bidi_property::R, false},
            {0, 0, bidi_property::B, false},
            {0, 0, bidi_property::AN, false},
        };

        auto runs = find_all_runs(paes.begin(), paes.end());
        auto run_sequences = find_run_sequences(paes, runs);
        EXPECT_EQ(run_sequences.size(), 1u);

        n1(run_sequences[0]);

        auto it = run_sequences[0].begin();
        EXPECT_EQ(
            std::distance(run_sequences[0].begin(), run_sequences[0].end()), 3);
        EXPECT_EQ(it->prop_, bidi_property::R);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::R);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::AN);
    }
    {
        props_and_embeddings_t paes = {
            {0, 0, bidi_property::R, false},
            {0, 0, bidi_property::B, false},
            {0, 0, bidi_property::EN, false},
        };

        auto runs = find_all_runs(paes.begin(), paes.end());
        auto run_sequences = find_run_sequences(paes, runs);
        EXPECT_EQ(run_sequences.size(), 1u);

        n1(run_sequences[0]);

        auto it = run_sequences[0].begin();
        EXPECT_EQ(
            std::distance(run_sequences[0].begin(), run_sequences[0].end()), 3);
        EXPECT_EQ(it->prop_, bidi_property::R);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::R);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::EN);
    }
    {
        props_and_embeddings_t paes = {
            {0, 0, bidi_property::AN, false},
            {0, 0, bidi_property::B, false},
            {0, 0, bidi_property::R, false},
        };

        auto runs = find_all_runs(paes.begin(), paes.end());
        auto run_sequences = find_run_sequences(paes, runs);
        EXPECT_EQ(run_sequences.size(), 1u);

        n1(run_sequences[0]);

        auto it = run_sequences[0].begin();
        EXPECT_EQ(
            std::distance(run_sequences[0].begin(), run_sequences[0].end()), 3);
        EXPECT_EQ(it->prop_, bidi_property::AN);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::R);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::R);
    }
    {
        props_and_embeddings_t paes = {
            {0, 0, bidi_property::AN, false},
            {0, 0, bidi_property::B, false},
            {0, 0, bidi_property::AN, false},
        };

        auto runs = find_all_runs(paes.begin(), paes.end());
        auto run_sequences = find_run_sequences(paes, runs);
        EXPECT_EQ(run_sequences.size(), 1u);

        n1(run_sequences[0]);

        auto it = run_sequences[0].begin();
        EXPECT_EQ(
            std::distance(run_sequences[0].begin(), run_sequences[0].end()), 3);
        EXPECT_EQ(it->prop_, bidi_property::AN);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::R);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::AN);
    }
    {
        props_and_embeddings_t paes = {
            {0, 0, bidi_property::AN, false},
            {0, 0, bidi_property::B, false},
            {0, 0, bidi_property::EN, false},
        };

        auto runs = find_all_runs(paes.begin(), paes.end());
        auto run_sequences = find_run_sequences(paes, runs);
        EXPECT_EQ(run_sequences.size(), 1u);

        n1(run_sequences[0]);

        auto it = run_sequences[0].begin();
        EXPECT_EQ(
            std::distance(run_sequences[0].begin(), run_sequences[0].end()), 3);
        EXPECT_EQ(it->prop_, bidi_property::AN);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::R);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::EN);
    }
    {
        props_and_embeddings_t paes = {
            {0, 0, bidi_property::EN, false},
            {0, 0, bidi_property::B, false},
            {0, 0, bidi_property::R, false},
        };

        auto runs = find_all_runs(paes.begin(), paes.end());
        auto run_sequences = find_run_sequences(paes, runs);
        EXPECT_EQ(run_sequences.size(), 1u);

        n1(run_sequences[0]);

        auto it = run_sequences[0].begin();
        EXPECT_EQ(
            std::distance(run_sequences[0].begin(), run_sequences[0].end()), 3);
        EXPECT_EQ(it->prop_, bidi_property::EN);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::R);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::R);
    }
    {
        props_and_embeddings_t paes = {
            {0, 0, bidi_property::EN, false},
            {0, 0, bidi_property::B, false},
            {0, 0, bidi_property::AN, false},
        };

        auto runs = find_all_runs(paes.begin(), paes.end());
        auto run_sequences = find_run_sequences(paes, runs);
        EXPECT_EQ(run_sequences.size(), 1u);

        n1(run_sequences[0]);

        auto it = run_sequences[0].begin();
        EXPECT_EQ(
            std::distance(run_sequences[0].begin(), run_sequences[0].end()), 3);
        EXPECT_EQ(it->prop_, bidi_property::EN);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::R);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::AN);
    }
    {
        props_and_embeddings_t paes = {
            {0, 0, bidi_property::EN, false},
            {0, 0, bidi_property::B, false},
            {0, 0, bidi_property::EN, false},
        };

        auto runs = find_all_runs(paes.begin(), paes.end());
        auto run_sequences = find_run_sequences(paes, runs);
        EXPECT_EQ(run_sequences.size(), 1u);

        n1(run_sequences[0]);

        auto it = run_sequences[0].begin();
        EXPECT_EQ(
            std::distance(run_sequences[0].begin(), run_sequences[0].end()), 3);
        EXPECT_EQ(it->prop_, bidi_property::EN);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::R);
        ++it;
        EXPECT_EQ(it->prop_, bidi_property::EN);
    }
}

TEST(detail_bidi, l2_)
{
    {
        props_and_embeddings_t paes = {
            {'c', 0, bidi_property::L, false},
            {'a', 0, bidi_property::L, false},
            {'r', 0, bidi_property::L, false},
            {' ', 0, bidi_property::WS, false},
            {'m', 0, bidi_property::L, false},
            {'e', 0, bidi_property::L, false},
            {'a', 0, bidi_property::L, false},
            {'n', 0, bidi_property::L, false},
            {'s', 0, bidi_property::L, false},
            {' ', 0, bidi_property::WS, false},
            {'C', 1, bidi_property::R, false},
            {'A', 1, bidi_property::R, false},
            {'R', 1, bidi_property::R, false},
            {'.', 0, bidi_property::CS, false},
        };

        auto runs = find_all_runs(paes.begin(), paes.end());
        reordered_runs_t reordered_runs = l2(runs);

        string result;
        uint32_t cps[1] = {0};
        for (auto run : reordered_runs) {
            if (run.reversed()) {
                for (auto it = run.rbegin(), end = run.rend(); it != end;
                     ++it) {
                    cps[0] = it->cp_;
                    result += to_string(cps, cps + 1);
                }
            } else {
                for (auto pae : run) {
                    cps[0] = pae.cp_;
                    result += to_string(cps, cps + 1);
                }
            }
        }

        EXPECT_EQ(result, "car means RAC.");
    }
    {
        props_and_embeddings_t paes = {
            {'<', 0, bidi_property::RLI, false},
            {'c', 2, bidi_property::L, false},
            {'a', 2, bidi_property::L, false},
            {'r', 2, bidi_property::L, false},
            {' ', 1, bidi_property::WS, false},
            {'M', 1, bidi_property::R, false},
            {'E', 1, bidi_property::R, false},
            {'A', 1, bidi_property::R, false},
            {'N', 1, bidi_property::R, false},
            {'S', 1, bidi_property::R, false},
            {' ', 1, bidi_property::WS, false},
            {'C', 1, bidi_property::R, false},
            {'A', 1, bidi_property::R, false},
            {'R', 1, bidi_property::R, false},
            {'.', 1, bidi_property::CS, false},
            {'=', 0, bidi_property::PDI, false},
        };

        auto runs = find_all_runs(paes.begin(), paes.end());
        reordered_runs_t reordered_runs = l2(runs);

        string result;
        uint32_t cps[1] = {0};
        for (auto run : reordered_runs) {
            if (run.reversed()) {
                for (auto it = run.rbegin(), end = run.rend(); it != end;
                     ++it) {
                    cps[0] = it->cp_;
                    result += to_string(cps, cps + 1);
                }
            } else {
                for (auto pae : run) {
                    cps[0] = pae.cp_;
                    result += to_string(cps, cps + 1);
                }
            }
        }

        EXPECT_EQ(result, "<.RAC SNAEM car=");
    }
    {
        props_and_embeddings_t paes = {
            {'h', 0, bidi_property::L, false},
            {'e', 0, bidi_property::L, false},
            {' ', 0, bidi_property::WS, false},
            {'s', 0, bidi_property::L, false},
            {'a', 0, bidi_property::L, false},
            {'i', 0, bidi_property::L, false},
            {'d', 0, bidi_property::L, false},
            {' ', 0, bidi_property::WS, false},
            {0x201c, 0, bidi_property::ON, false}, // ” U+201C Left Double Quotation
            {'<', 0, bidi_property::RLI, false},
            {'c', 2, bidi_property::L, false},
            {'a', 2, bidi_property::L, false},
            {'r', 2, bidi_property::L, false},
            {' ', 1, bidi_property::WS, false},
            {'M', 1, bidi_property::R, false},
            {'E', 1, bidi_property::R, false},
            {'A', 1, bidi_property::R, false},
            {'N', 1, bidi_property::R, false},
            {'S', 1, bidi_property::R, false},
            {' ', 1, bidi_property::WS, false},
            {'C', 1, bidi_property::R, false},
            {'A', 1, bidi_property::R, false},
            {'R', 1, bidi_property::R, false},
            {'=', 0, bidi_property::PDI, false},
            {'.', 0, bidi_property::CS, false},
            {0x201d, 0, bidi_property::ON, false}, // ” U+201D Right Double Quotation
            {' ', 0, bidi_property::WS, false},
            {0x201c, 0, bidi_property::ON, false},
            {'<', 0, bidi_property::RLI, false},
            {'I', 1, bidi_property::R, false},
            {'T', 1, bidi_property::R, false},
            {' ', 1, bidi_property::WS, false},
            {'D', 1, bidi_property::R, false},
            {'O', 1, bidi_property::R, false},
            {'E', 1, bidi_property::R, false},
            {'S', 1, bidi_property::R, false},
            {'=', 0, bidi_property::PDI, false},
            {',', 0, bidi_property::CS, false},
            {0x201d, 0, bidi_property::ON, false},
            {' ', 0, bidi_property::WS, false},
            {'s', 0, bidi_property::L, false},
            {'h', 0, bidi_property::L, false},
            {'e', 0, bidi_property::L, false},
            {' ', 0, bidi_property::WS, false},
            {'a', 0, bidi_property::L, false},
            {'g', 0, bidi_property::L, false},
            {'r', 0, bidi_property::L, false},
            {'e', 0, bidi_property::L, false},
            {'e', 0, bidi_property::L, false},
            {'d', 0, bidi_property::L, false},
            {'.', 0, bidi_property::CS, false},
        };

        auto runs = find_all_runs(paes.begin(), paes.end());
        reordered_runs_t reordered_runs = l2(runs);

        string result;
        uint32_t cps[1] = {0};
        for (auto run : reordered_runs) {
            if (run.reversed()) {
                for (auto it = run.rbegin(), end = run.rend(); it != end;
                     ++it) {
                    cps[0] = it->cp_;
                    result += to_string(cps, cps + 1);
                }
            } else {
                for (auto pae : run) {
                    cps[0] = pae.cp_;
                    result += to_string(cps, cps + 1);
                }
            }
        }

        EXPECT_EQ(result, string("he said \u201c<RAC SNAEM car=.\u201d \u201c<SEOD TI=,\u201d she agreed."));
    }
    {
        props_and_embeddings_t paes = {
            {'D', 1, bidi_property::R, false},
            {'I', 1, bidi_property::R, false},
            {'D', 1, bidi_property::R, false},
            {' ', 1, bidi_property::WS, false},
            {'Y', 1, bidi_property::R, false},
            {'O', 1, bidi_property::R, false},
            {'U', 1, bidi_property::R, false},
            {' ', 1, bidi_property::WS, false},
            {'S', 1, bidi_property::R, false},
            {'A', 1, bidi_property::R, false},
            {'Y', 1, bidi_property::R, false},
            {' ', 1, bidi_property::WS, false},
            {0x2019, 1, bidi_property::ON, false}, // U+2018 Single Left Quotation
            {'>', 1, bidi_property::LRI, false},
            {'h', 2, bidi_property::L, false},
            {'e', 2, bidi_property::L, false},
            {' ', 2, bidi_property::WS, false},
            {'s', 2, bidi_property::L, false},
            {'a', 2, bidi_property::L, false},
            {'i', 2, bidi_property::L, false},
            {'d', 2, bidi_property::L, false},
            {' ', 2, bidi_property::WS, false},
            {0x201c, 2, bidi_property::ON, false},
            {'<', 2, bidi_property::RLI, false},
            {'c', 4, bidi_property::L, false},
            {'a', 4, bidi_property::L, false},
            {'r', 4, bidi_property::L, false},
            {' ', 3, bidi_property::WS, false},
            {'M', 3, bidi_property::R, false},
            {'E', 3, bidi_property::R, false},
            {'A', 3, bidi_property::R, false},
            {'N', 3, bidi_property::R, false},
            {'S', 3, bidi_property::R, false},
            {' ', 3, bidi_property::WS, false},
            {'C', 3, bidi_property::R, false},
            {'A', 3, bidi_property::R, false},
            {'R', 3, bidi_property::R, false},
            {'=', 2, bidi_property::PDI, false},
            {0x201d, 2, bidi_property::ON, false},
            {'=', 1, bidi_property::PDI, false},
            {0x2018, 1, bidi_property::ON, false}, // U+2019 Single Right Quotation
            {'?', 1, bidi_property::ON, false},
        };

        auto runs = find_all_runs(paes.begin(), paes.end());
        reordered_runs_t reordered_runs = l2(runs);

        string result;
        uint32_t cps[1] = {0};
        for (auto run : reordered_runs) {
            if (run.reversed()) {
                for (auto it = run.rbegin(), end = run.rend(); it != end;
                     ++it) {
                    cps[0] = it->cp_;
                    result += to_string(cps, cps + 1);
                }
            } else {
                for (auto pae : run) {
                    cps[0] = pae.cp_;
                    result += to_string(cps, cps + 1);
                }
            }
        }

        EXPECT_EQ(result, string("?\u2018=he said \u201c<RAC SNAEM car=\u201d>\u2019 YAS UOY DID"));
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
