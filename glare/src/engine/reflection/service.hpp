#pragma once

#include "empty.hpp"

#include <engine/service.hpp>

#include <type_traits>

namespace engine
{
	template <typename T>
    auto engine_service_type(bool sync_context=true)
    {
        auto type = engine_empty_type<T>(sync_context)
            .prop("service"_hs)
        ;

        if constexpr ((!std::is_same_v<T, Service>) && (std::is_base_of_v<Service, T>))
        {
            type = type
                .base<Service>()

                .template func<&impl::from_base_ptr<T, Service>>("dynamic_cast"_hs)
            ;
        }

        return type;
    }
}