#pragma once

#include "component_extensions.hpp"

#include <engine/types.hpp>

#include <engine/meta/meta.hpp>
#include <engine/meta/short_name.hpp>

#include <engine/history/components/history_component.hpp>

#include <type_traits>

namespace engine
{
	namespace impl
	{
		template <typename T>
        auto history_component_meta_type(auto history_type)
        {
            using namespace engine::literals;

            using history_component_t = HistoryComponent<T>;
            using history_log_t = typename history_component_t::LogType; // util::HistoryLog<T>;

            history_type = history_type
                .prop("history"_hs)
                .prop("history_component"_hs)
                .prop("component"_hs)
            ;

            /*
                NOTE: This check is a workaround for EnTT's `is_dynamic_sequence_container` trait,
                which checks for the existence of the `clear` member-function to enable
                manipulation of dynamic sequence containers (e.g. `std::vector`).
            
                Although `clear` isn't necessarily a problem for many types, `resize` can be,
                since it requires default construction and move construction.
            
                EnTT currently allows `resize` to be used if `is_dynamic_sequence_container` reports true,
                so for this reason we have to avoid reflecting the `history` member
                if we're unable to resize the underlying container.
            
                 This makes sense at a meta-level as well, since `truncate` (which uses `resize` internally)
                is a critical feature of `util::HistoryLog`.
            */
            if constexpr (std::is_default_constructible_v<T> && std::is_move_constructible_v<T>)
            {
                history_type = history_type
                    .func<static_cast<bool(history_component_t::*)(const T&)>(&history_component_t::store)>("store"_hs)
                    .func<static_cast<bool(history_component_t::*)(Registry&, Entity)>(&history_component_t::store)>("store"_hs)

                    .func<static_cast<bool(history_component_t::*)(const T&)>(&history_component_t::store_back)>("store_back"_hs)
                    .func<static_cast<bool(history_component_t::*)(Registry&, Entity)>(&history_component_t::store_back)>("store_back"_hs)

                    .func<static_cast<bool(history_component_t::*)(T&)>(&history_component_t::undo)>("undo"_hs)
                    .func<static_cast<bool(history_component_t::*)(Registry&, Entity)>(&history_component_t::undo)>("undo"_hs)

                    .func<static_cast<bool(history_component_t::*)(T&)>(&history_component_t::redo)>("redo"_hs)
                    .func<static_cast<bool(history_component_t::*)(Registry&, Entity)>(&history_component_t::redo)>("redo"_hs)

                    .func<static_cast<bool(history_component_t::*)(bool)>(&history_component_t::truncate_back)>("truncate_back"_hs)
                    .func<static_cast<bool(history_component_t::*)()>(&history_component_t::truncate_back)>("truncate_back"_hs)

                    .func<&history_component_t::truncate>("truncate"_hs)

                    .func<&history_component_t::cursor_out_of_bounds>("cursor_out_of_bounds"_hs)
                    .func<&history_component_t::get_snapshot>("get_snapshot"_hs)
                ;

                history_type = history_type
                    .data<&history_component_t::history>("history"_hs)
                ;

                history_type = history_type
                    .data<nullptr, &history_component_t::get_active_snapshot>("active_snapshot"_hs)
                    .data<nullptr, &history_component_t::get_cursor>("cursor"_hs)
                    .data<nullptr, &history_component_t::size>("size"_hs)
                    .data<nullptr, &history_component_t::empty>("empty"_hs)
                    .data<nullptr, &history_component_t::can_undo>("can_undo"_hs)
                    .data<nullptr, &history_component_t::can_redo>("can_redo"_hs)
                    .data<nullptr, &history_component_t::can_truncate>("can_truncate"_hs)
                    .data<nullptr, &history_component_t::can_store>("can_store"_hs)
                    .data<nullptr, &history_component_t::can_clear>("can_clear"_hs)
                    .data<nullptr, &history_component_t::can_copy_value>("can_copy_value"_hs)
                    .data<nullptr, &history_component_t::can_move_value>("can_move_value"_hs)
                    .data<nullptr, &history_component_t::has_default_cursor>("has_default_cursor"_hs)
                    .data<nullptr, &history_component_t::has_live_value_cursor>("has_live_value_cursor"_hs)
                    .data<nullptr, &history_component_t::get_default_cursor>("default_cursor"_hs)
                    .data<nullptr, &history_component_t::get_live_value_cursor>("live_value_cursor"_hs)
                ;
            }

            return history_type;
        }
	}

    template <typename T>
    auto engine_history_component_type(bool sync_context=true)
    {
        using namespace engine::literals;

        using history_component_t = HistoryComponent<T>;

        if (sync_context)
        {
            // Ensure that we're using the correct context.
            sync_reflection_context();
        }

        auto history_type = entt::meta<history_component_t>()
            .type(history_component_short_name_hash<T>())

            .func<&impl::get_component_type_id_impl<T>>("get_component_type_id"_hs)

            //.data<nullptr, &impl::get_component_type_id_impl<T>>("component_type_id"_hs)
            //.prop<get_component_type_id_impl<T>()>("component_type_id"_hs)
        ;

        return impl::history_component_meta_type<T>(history_type);
    }
}