#ifndef EDITOR_APP_STATE_HPP
#define EDITOR_APP_STATE_HPP

#include "buffer.hpp"
#include "event.hpp"
#include "key_mappings.hpp"

#include <boost/optional.hpp>


struct app_state_t
{
    buffer_t buffer_;
    key_map_t key_map_;
    key_sequence_t input_seq_;
};

boost::optional<app_state_t> update(app_state_t state, event_t event);

#endif
