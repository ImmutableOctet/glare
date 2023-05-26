#pragma once

#include "common_extensions.hpp"

#include <engine/service.hpp>
#include <engine/meta/types.hpp>

#include <utility>

namespace engine::impl
{
	template <typename EventType>
    void trigger_event_from_meta_any(Service& service, MetaAny event_instance)
    {
        if (auto raw_value = from_meta<EventType>(event_instance))
        {
            return service.event<EventType>(std::move(*raw_value)); // *raw_value
        }
        else
        {
            //throw std::exception("Invalid value specified; unable to trigger event.");
        }
    }
}