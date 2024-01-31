#pragma once

#include "context.hpp"
#include "operators.hpp"
#include "optional_extensions.hpp"

#include <engine/types.hpp>

#include <engine/meta/meta.hpp>
#include <engine/meta/indirection.hpp>
#include <engine/meta/short_name.hpp>
#include <engine/meta/hash.hpp>

#include <optional>
#include <type_traits>

namespace engine
{
	namespace impl
	{
		template <typename T>
        auto optional_meta_type(auto opt_type, bool sync_context=true)
        {
            if (sync_context)
            {
                // Ensure that we're using the correct context.
                sync_reflection_context();
            }

            using optional_t = std::optional<T>;

            opt_type = opt_type
                .prop("optional"_hs)

                // NOTE: Added as both a function and data-member.
                .data<nullptr, &optional_t::has_value>("has_value"_hs)

                .data<nullptr, &impl::optional_to_ref<T>, entt::as_ref_t>("value"_hs) // entt::as_cref_t

                .func<&impl::type_id_from_optional<T>>("type_id_from_optional"_hs)
                .func<&impl::type_from_optional<T>>("type_from_optional"_hs)
                .func<&impl::optional_get_type_impl<T>>("get_type"_hs)
                .func<&impl::optional_value_or_impl<T>>("value_or"_hs)

                // NOTE: Added as both a function and data-member.
                .func<&optional_t::has_value>("has_value"_hs)

                .func<&try_get_underlying_type<optional_t>>("try_get_underlying_type"_hs)

                .func<&impl::optional_to_const_ref<T>, entt::as_cref_t>("*operator"_hs)

                .func<&impl::optional_resolve_impl<T>>("operator()"_hs)
            ;

            if constexpr (std::is_copy_constructible_v<T>)
            {
                opt_type = opt_type
                    .conv<&impl::optional_to_value<T>>()
                    .ctor<T>(); // const T&
                ;
            }

            /*
            if constexpr (std::is_move_constructible_v<T>)
            {
                opt_type = opt_type.ctor<T&&>();
            }
            */

            opt_type = define_boolean_operators<optional_t>(opt_type);

            return opt_type;
        }
	}

    template <typename T>
    auto engine_optional_type(bool sync_context=true)
    {
        using optional_t = std::optional<T>;

        if (sync_context)
        {
            // Ensure that we're using the correct context.
            sync_reflection_context();
        }

        auto opt_type = entt::meta<optional_t>()
            .type(optional_short_name_hash<T>())
        ;

        return impl::optional_meta_type<T>(opt_type, false);
    }

    template <typename T>
    auto custom_optional_type(MetaTypeID type_id, bool sync_context=true)
    {
        using optional_t = std::optional<T>;

        if (sync_context)
        {
            // Ensure that we're using the correct context.
            sync_reflection_context();
        }

        auto opt_type = entt::meta<optional_t>()
            .type(type_id)
        ;

        return impl::optional_meta_type<T>(opt_type, false);
    }

    template <typename T>
    auto custom_optional_type(std::string_view underlying_type_name, bool sync_context=true)
    {
        const auto opt_type_id = hash(optional_name(underlying_type_name));

        return custom_optional_type<T>(opt_type_id.value(), sync_context);
    }
}