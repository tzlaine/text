#include <boost/text/table_serialization.hpp>

#include <boost/endian/buffers.hpp>
#include <boost/filesystem/fstream.hpp>


#define BOOST_TEXT_ASSERT_ALL_THE_THINGS 0

namespace boost { namespace text {

    namespace {
        enum current_version : int { serialization_version = 0 };

        struct serialized_collation_element_t
        {
            endian::little_uint32_buf_t l1_;
            endian::little_uint16_buf_t l2_;
            endian::little_uint16_buf_t l3_;
            endian::little_uint32_buf_t l4_;
        };

        inline serialized_collation_element_t
        convert(detail::collation_element ce)
        {
            serialized_collation_element_t retval;
            retval.l1_ = ce.l1_;
            retval.l2_ = ce.l2_;
            retval.l3_ = ce.l3_;
            retval.l4_ = ce.l4_;
            return retval;
        }

        inline detail::collation_element
        convert(serialized_collation_element_t sce)
        {
            detail::collation_element retval;
            retval.l1_ = sce.l1_.value();
            retval.l2_ = sce.l2_.value();
            retval.l3_ = sce.l3_.value();
            retval.l4_ = sce.l4_.value();
            return retval;
        }

        struct serialized_nonsimple_script_reorder_t
        {
            serialized_collation_element_t first_;
            serialized_collation_element_t last_;
            endian::little_uint32_buf_t lead_byte_;
        };

        inline serialized_nonsimple_script_reorder_t
        convert(detail::nonsimple_script_reorder nsr)
        {
            serialized_nonsimple_script_reorder_t retval;
            retval.first_ = convert(nsr.first_);
            retval.last_ = convert(nsr.last_);
            retval.lead_byte_ = nsr.lead_byte_;
            return retval;
        }

        inline detail::nonsimple_script_reorder
        convert(serialized_nonsimple_script_reorder_t snsr)
        {
            detail::nonsimple_script_reorder retval;
            retval.first_ = convert(snsr.first_);
            retval.last_ = convert(snsr.last_);
            retval.lead_byte_ = snsr.lead_byte_.value();
            return retval;
        }

        struct header_t
        {
            endian::little_int32_buf_t version_;

            endian::little_int32_buf_t collation_elements_;
            endian::little_int32_buf_t trie_;
            endian::little_int32_buf_t nonsimple_reorders_;
            endian::little_int32_buf_t simple_reorders_;

            endian::little_uint8_buf_t have_strength_;
            endian::little_uint8_buf_t strength_;
            endian::little_uint8_buf_t have_weighting_;
            endian::little_uint8_buf_t weighting_;
            endian::little_uint8_buf_t have_l2_order_;
            endian::little_uint8_buf_t l2_order_;
            endian::little_uint8_buf_t have_case_level_;
            endian::little_uint8_buf_t case_level_;
            endian::little_uint8_buf_t have_case_first_;
            endian::little_uint8_buf_t case_first_;
        };

        struct trie_element_header_t
        {
            endian::little_int32_buf_t key_size_;
            endian::little_int32_buf_t value_size_;
        };

        inline void load_trie(
            filesystem::ifstream & ifs,
            detail::collation_trie_t & trie,
            int size)
        {
            for (int i = 0; i < size; ++i) {
                trie_element_header_t header;
                ifs.read(reinterpret_cast<char *>(&header), sizeof(header));
                if (!ifs)
                    throw serialization_error("Unable to read trie node.");
                detail::collation_trie_key<32> key;
                key.size_ = header.key_size_.value();
                for (int i = 0, end = key.size_; i < end; ++i) {
                    endian::little_uint32_buf_t cp;
                    ifs.read(reinterpret_cast<char *>(&cp), sizeof(cp));
                    key.cps_.values_[i] = cp.value();
                }
                endian::little_uint16_buf_t first;
                endian::little_uint16_buf_t last;
                ifs.read(reinterpret_cast<char *>(&first), sizeof(first));
                ifs.read(reinterpret_cast<char *>(&last), sizeof(last));
                detail::collation_elements const value{first.value(),
                                                       last.value()};
                trie.insert(key, value);
            }
        }

        inline void save_trie(
            filesystem::ofstream & ofs, detail::collation_trie_t const & trie)
        {
            for (auto const & element : trie) {
                trie_element_header_t header;
                header.key_size_ = element.key.size_;
                header.value_size_ = element.value.size();
                ofs.write(
                    reinterpret_cast<char const *>(&header), sizeof(header));
                for (int i = 0, end = element.key.size_; i < end; ++i) {
                    endian::little_uint32_buf_t cp;
                    cp = element.key.cps_.values_[i];
                    ofs.write(reinterpret_cast<char const *>(&cp), sizeof(cp));
                }
                endian::little_uint16_buf_t first;
                first = element.value.first_;
                endian::little_uint16_buf_t last;
                last = element.value.last_;
                ofs.write(
                    reinterpret_cast<char const *>(&first), sizeof(first));
                ofs.write(reinterpret_cast<char const *>(&last), sizeof(last));
            }
        }

        inline void load_collation_elements(
            filesystem::ifstream & ifs,
            std::vector<detail::collation_element> & collation_element_vec,
            detail::collation_element const *& collation_elements,
            int size)
        {
            if (size) {
                collation_element_vec.resize(size);
                for (int i = 0; i < size; ++i) {
                    serialized_collation_element_t sce;
                    ifs.read(reinterpret_cast<char *>(&sce), sizeof(sce));
                    collation_element_vec[i] = convert(sce);
                }
            } else {
                collation_elements = detail::g_collation_elements_first;
            }
        }

        inline void save_collation_elements(
            filesystem::ofstream & ofs,
            std::vector<detail::collation_element> const &
                collation_element_vec,
            detail::collation_element const * collation_elements)
        {
            for (auto ce : collation_element_vec) {
                serialized_collation_element_t const sce = convert(ce);
                ofs.write(reinterpret_cast<char const *>(&sce), sizeof(sce));
            }
        }

        inline void load_nonsimple_reorders(
            filesystem::ifstream & ifs,
            detail::nonsimple_reorders_t & nonsimple_reorders,
            int size)
        {
            nonsimple_reorders.resize(size);
            for (int i = 0; i < size; ++i) {
                serialized_nonsimple_script_reorder_t snsr;
                ifs.read(reinterpret_cast<char *>(&snsr), sizeof(snsr));
                nonsimple_reorders[i] = convert(snsr);
            }
        }

        inline void save_nonsimple_reorders(
            filesystem::ofstream & ofs,
            detail::nonsimple_reorders_t const & nonsimple_reorders)
        {
            for (auto reorder : nonsimple_reorders) {
                serialized_nonsimple_script_reorder_t const snsr =
                    convert(reorder);
                ofs.write(reinterpret_cast<char const *>(&snsr), sizeof(snsr));
            }
        }

        inline void load_simple_reorders(
            filesystem::ifstream & ifs,
            std::array<uint32_t, 256> & simple_reorders,
            int size)
        {
            assert(size == 256);
            for (int i = 0; i < size; ++i) {
                endian::little_uint32_buf_t r;
                ifs.read(reinterpret_cast<char *>(&r), sizeof(r));
                simple_reorders[i] = r.value();
            }
        }

        inline void save_simple_reorders(
            filesystem::ofstream & ofs,
            std::array<uint32_t, 256> const & simple_reorders)
        {
            for (auto reorder : simple_reorders) {
                endian::little_uint32_buf_t const r{reorder};
                ofs.write(reinterpret_cast<char const *>(&r), sizeof(r));
            }
        }
    }

    void save_table(
        collation_table const & table_proper, filesystem::path const & path)
    {
        auto const & table = *table_proper.data_;

        header_t header;
        header.version_ = serialization_version;
        header.collation_elements_ = int(table.collation_element_vec_.size());
        header.trie_ = int(table.trie_.size());
        header.nonsimple_reorders_ = int(table.nonsimple_reorders_.size());
        header.simple_reorders_ = int(table.simple_reorders_.size());
        header.have_strength_ = !!table.strength_;
        if (table.strength_)
            header.strength_ = uint8_t(*table.strength_);
        header.have_weighting_ = !!table.weighting_;
        if (table.weighting_)
            header.weighting_ = uint8_t(*table.weighting_);
        header.have_l2_order_ = !!table.l2_order_;
        if (table.l2_order_)
            header.l2_order_ = uint8_t(*table.l2_order_);
        header.have_case_level_ = !!table.case_level_;
        if (table.case_level_)
            header.case_level_ = uint8_t(*table.case_level_);
        header.have_case_first_ = !!table.case_first_;
        if (table.case_first_)
            header.case_first_ = uint8_t(*table.case_first_);

        filesystem::ofstream ofs(path, std::ios_base::binary);
        ofs.write(reinterpret_cast<char const *>(&header), sizeof(header));

        save_collation_elements(
            ofs, table.collation_element_vec_, table.collation_elements_);

        save_nonsimple_reorders(ofs, table.nonsimple_reorders_);

        save_simple_reorders(ofs, table.simple_reorders_);

        save_trie(ofs, table.trie_);

        if (!ofs)
            throw serialization_error("Unable to write table to file.");

#if BOOST_TEXT_ASSERT_ALL_THE_THINGS
        ofs.close();
        filesystem::ifstream ifs(path, std::ios_base::binary);

        header_t loaded_header;
        ifs.read(
            reinterpret_cast<char *>(&loaded_header), sizeof(loaded_header));
        assert(loaded_header.version_.value() == header.version_.value());
        assert(
            loaded_header.collation_elements_.value() ==
            header.collation_elements_.value());
        assert(loaded_header.trie_.value() == header.trie_.value());
        assert(
            loaded_header.nonsimple_reorders_.value() ==
            header.nonsimple_reorders_.value());
        assert(
            loaded_header.simple_reorders_.value() ==
            header.simple_reorders_.value());
        assert(
            loaded_header.have_strength_.value() ==
            header.have_strength_.value());
        assert(loaded_header.strength_.value() == header.strength_.value());
        assert(
            loaded_header.have_weighting_.value() ==
            header.have_weighting_.value());
        assert(loaded_header.weighting_.value() == header.weighting_.value());
        assert(
            loaded_header.have_l2_order_.value() ==
            header.have_l2_order_.value());
        assert(loaded_header.l2_order_.value() == header.l2_order_.value());
        assert(
            loaded_header.have_case_level_.value() ==
            header.have_case_level_.value());
        assert(loaded_header.case_level_.value() == header.case_level_.value());
        assert(
            loaded_header.have_case_first_.value() ==
            header.have_case_first_.value());
        assert(loaded_header.case_first_.value() == header.case_first_.value());

        std::vector<detail::collation_element> collation_element_vec;
        detail::collation_element const * collation_elements = nullptr;
        load_collation_elements(
            ifs,
            collation_element_vec,
            collation_elements,
            header.collation_elements_.value());
        assert(collation_element_vec == table.collation_element_vec_);
        assert(collation_elements == table.collation_elements_);

        detail::nonsimple_reorders_t nonsimple_reorders;
        load_nonsimple_reorders(
            ifs, nonsimple_reorders, header.nonsimple_reorders_.value());
        assert(nonsimple_reorders == table.nonsimple_reorders_);

        std::array<uint32_t, 256> simple_reorders;
        load_simple_reorders(
            ifs, simple_reorders, header.simple_reorders_.value());
        assert(simple_reorders == table.simple_reorders_);

        detail::collation_trie_t trie;
        load_trie(ifs, trie, header.trie_.value());
        assert(trie == table.trie_);
#endif
    }

    collation_table load_table(filesystem::path const & path)
    {
        collation_table retval;

        auto & table = *retval.data_;

        filesystem::ifstream ifs(path, std::ios_base::binary);

        header_t header;
        ifs.read(reinterpret_cast<char *>(&header), sizeof(header));

        if (!ifs)
            throw serialization_error("Unable to read table file header.");
        if (header.version_.value() != serialization_version)
            throw serialization_error("Unknown version number in table file.");

        load_collation_elements(
            ifs,
            table.collation_element_vec_,
            table.collation_elements_,
            header.collation_elements_.value());

        load_nonsimple_reorders(
            ifs, table.nonsimple_reorders_, header.nonsimple_reorders_.value());

        load_simple_reorders(
            ifs, table.simple_reorders_, header.simple_reorders_.value());

        if (header.have_strength_.value())
            table.strength_ = collation_strength(header.strength_.value());
        if (header.have_weighting_.value())
            table.weighting_ = variable_weighting(header.weighting_.value());
        if (header.have_l2_order_.value())
            table.l2_order_ = l2_weight_order(header.l2_order_.value());
        if (header.have_case_level_.value())
            table.case_level_ = case_level_t(header.case_level_.value());
        if (header.have_case_first_.value())
            table.case_first_ = case_first_t(header.case_first_.value());

        load_trie(ifs, table.trie_, header.trie_.value());

        if (!ifs)
            throw serialization_error("Unable to read from table file.");

        return retval;
    }

}}
