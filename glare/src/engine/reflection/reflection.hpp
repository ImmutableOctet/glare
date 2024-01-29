#pragma once

// This is an internal header for `engine` module and should only be used by reflection implementation files.
// 
// This header defines general-purpose reflection facilities for systems, components, etc. using EnTT's meta-type system.
// For consuming or instantiating reflection data, it is recommended that you directly use the `meta` header instead.

#include "meta_type_reflection_config.hpp"

#include "context.hpp"
#include "function.hpp"
#include "operators.hpp"
#include "indirection.hpp"
#include "json_bindings.hpp"
#include "binary_bindings.hpp"

#include "common_extensions.hpp"
#include "component_extensions.hpp"
#include "optional_extensions.hpp"
#include "json_extensions.hpp"
#include "service_extensions.hpp"

#include <engine/types.hpp>

//#include <engine/meta/meta.hpp>
#include <engine/meta/types.hpp>
#include <engine/meta/enum.hpp>
#include <engine/meta/hash.hpp>
#include <engine/meta/indirection.hpp>
#include <engine/meta/serial.hpp>
#include <engine/meta/traits.hpp>
#include <engine/meta/short_name.hpp>
#include <engine/meta/cast.hpp>

#include <engine/meta/reflect_all.hpp>

#include <engine/meta/meta_type_descriptor.hpp>
#include <engine/meta/meta_event_listener.hpp>
#include <engine/meta/meta_evaluation_context.hpp>

#include <engine/service.hpp>
#include <engine/system_manager_interface.hpp>
#include <engine/command.hpp>

#include <engine/history/components/history_component.hpp>

#include <util/reflection.hpp>
#include <util/json.hpp>

#include <string_view>
#include <type_traits>
#include <utility>
#include <optional>
#include <tuple>
#include <stdexcept>
#include <cstddef>

// Debugging related:
#include <util/log.hpp>

// Declares a meta-type for a type with a single field.
#define REFLECT_SINGLE_FIELD_TYPE(type_name, field_name)                \
    engine::engine_meta_type<type_name>()                               \
		.data<&type_name::field_name>(entt::hashed_string(#field_name)) \
		.ctor<decltype(type_name::field_name)>();

#define GENERATE_SINGLE_FIELD_TYPE_REFLECTION(type_name, field_name) \
    template <>                                                      \
    void reflect<type_name>()                                        \
    {                                                                \
        REFLECT_SINGLE_FIELD_TYPE(type_name, field_name);            \
    }

// Declares a meta-type for a type, derived from another type, with a single field.
#define REFLECT_SINGLE_FIELD_DERIVED_TYPE(type_name, base_type_name, field_name) \
    engine::engine_meta_type<type_name>()                                        \
        .base<base_type_name>()                                                  \
		.data<&type_name::field_name>(entt::hashed_string(#field_name))

#define GENERATE_SINGLE_FIELD_DERIVED_TYPE_REFLECTION(type_name, base_type_name, field_name) \
    template <>                                                                              \
    void reflect<type_name>()                                                                \
    {                                                                                        \
        REFLECT_SINGLE_FIELD_DERIVED_TYPE(type_name, base_type_name, field_name);            \
    }

#define GENERATE_SINGLE_FIELD_COMPONENT_REFLECTION GENERATE_SINGLE_FIELD_TYPE_REFLECTION
#define REFLECT_SINGLE_FIELD_COMPONENT REFLECT_SINGLE_FIELD_TYPE

// Generates an empty `reflect` function for the specified `engine` type.
#define GENERATE_EMPTY_TYPE_REFLECTION(type_name) \
    template <>                                   \
    void reflect<type_name>()                     \
    {                                             \
        engine::engine_meta_type<type_name>();    \
    }

// Generates an empty `reflect` function for the specified `engine` type,
// where `type_name` is derived from `base_type_name`.
#define GENERATE_EMPTY_DERIVED_TYPE_REFLECTION(type_name, base_type_name) \
    template <>                                                           \
    void reflect<type_name>()                                             \
    {                                                                     \
        engine::engine_meta_type<type_name>()                             \
            .base<base_type_name>()                                       \
        ;                                                                 \
    }

namespace engine
{
	using namespace engine::literals;

    class Service;

    struct MetaEvaluationContext;

    struct MetaParsingInstructions;
    struct MetaTypeDescriptorFlags;

    // NOTE: In the default case of `T=void`, the overridden version of this template is used.
    // TODO: Look into best way to handle multiple calls to reflect. (This is currently only managed in `reflect_all`)
    template <typename T=void>
    void reflect()
    {
        if constexpr (std::is_enum_v<T>)
        {
            reflect_enum<T>();
        }
        else if constexpr (has_function_reflect<T, void>)
        {
            T::reflect();
        }
        else if constexpr (has_function_reflect<T, entt::meta_factory<T>>)
        {
            // TODO: Determine if it makes sense to forward the return-value to the caller.
            T::reflect();
        }
        else
        {
            static_assert(std::integral_constant<T, false>::value, "Reflection definition missing for type `T`.");
        }
    }

    // Aliases the default configuration of `reflect` to the `reflect_all` free-function.
    // 
    // TODO: Determine if this is the best option for a generalized `reflect`.
    template <>
    inline void reflect<void>()
    {
        reflect_all();
    }

    template <typename T>
    auto engine_empty_meta_type()
    {
        // Ensure that we're using the correct context.
        sync_reflection_context();

        auto type = entt::meta<T>()
            .type(short_name_hash<T>())
        ;

        return type;
    }

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

    template <typename T>
    auto optional_engine_meta_type(bool sync_context=true)
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

        return optional_meta_type<T>(opt_type, false);
    }

    template <typename T>
    auto history_component_meta_type(auto history_type, bool sync_context=true)
    {
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

    template <typename T>
    auto history_component_engine_meta_type(bool sync_context=true)
    {
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

        return history_component_meta_type<T>(history_type, false);
    }

    template <typename T>
    auto optional_custom_meta_type(MetaTypeID type_id, bool sync_context=true)
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

        return optional_meta_type<T>(opt_type, false);
    }

    template <typename T>
    auto optional_custom_meta_type(std::string_view underlying_type_name, bool sync_context=true)
    {
        const auto opt_type_id = hash(optional_name(underlying_type_name));

        return optional_custom_meta_type<T>(opt_type_id.value(), sync_context);
    }

    template <typename T, MetaTypeReflectionConfig config={}>
    auto define_standard_meta_type(auto type, bool sync_context=true)
    {
        if (sync_context)
        {
            // Ensure that we're using the correct context.
            sync_reflection_context();
        }

        type = type
            .template func<&impl::from_meta<T>>("from_meta"_hs)
            .template func<&short_name<T>>("get_type_name"_hs)
            .template func<&impl::trigger_event_from_meta_any<T>>("trigger_event_from_meta_any"_hs)
            .template func<&try_get_underlying_type<T>>("try_get_underlying_type"_hs)
        ;

        if constexpr (has_method_has_type_v<T, bool>)
        {
            type = type.template func<&T::has_type>("has_type"_hs);
        }

        if constexpr (has_method_get_type_v<T, MetaType>)
        {
            // NOTE: `get_type` is not guaranteed to be the same type as the result of `try_get_underlying_value`, or `try_get_underlying_type`.
            // For example, a `MetaFunctionCall` object can have several different types associated; 'self' type, 'return' type, etc.
            type = type.template func<&T::get_type>("get_type"_hs);
        }

        // NOTE: A type must be non-empty to qualify for usage as a component.
        if constexpr (!std::is_empty_v<T>)
        {
            type = type
                .template func<&impl::has_component<T>>("has_component"_hs)
                .template func<&impl::get_component<T>>("get_component"_hs)
                .template func<&impl::get_or_emplace_component<T>, entt::as_ref_t>("get_or_emplace_component"_hs)
                .template func<&impl::emplace_meta_component<T>, entt::as_ref_t>("emplace_meta_component"_hs)
                .template func<&impl::store_meta_component<T>>("store_meta_component"_hs)
                .template func<&impl::copy_meta_component<T>>("copy_meta_component"_hs)
                .template func<&impl::remove_component<T>>("remove_component"_hs)
                .template func<&impl::direct_patch_meta_component<T>, entt::as_ref_t>("direct_patch_meta_component"_hs)
                .template func<&impl::indirect_patch_meta_component<T>>("indirect_patch_meta_component"_hs)
                .template func<&impl::mark_component_as_patched<T>>("mark_component_as_patched"_hs)

			    .template func<&MetaEventListener::connect_component_listeners<T>>("connect_component_meta_events"_hs)
                .template func<&MetaEventListener::disconnect_component_listeners<T>>("disconnect_component_meta_events"_hs)
                .template func<&MetaEventListener::connect<T>>("connect_meta_event"_hs)
                .template func<&MetaEventListener::disconnect<T>>("disconnect_meta_event"_hs)
            ;

            if constexpr (std::is_default_constructible_v<T>)
            {
                type = type
                    .template func<&impl::emplace_default_component<T>>("emplace_default_component"_hs)
                    .template func<&impl::get_or_default_construct_component<T>>("get_or_default_construct_component"_hs)
                ;
            }

            if constexpr (config.generate_history_component_reflection)
            {
                if constexpr (std::is_copy_constructible_v<T>) // || (std::is_default_constructible_v<T> && std::is_copy_assignable_v<T>)
                {
                    history_component_engine_meta_type<T>(false);

                    type = type
                        .template func<&impl::get_history_component_type_id_impl<T>>("get_history_component_type_id"_hs)
                    ;

                    //"get_history_component_type_id"_hs
                }
            }
        }

        if constexpr (config.generate_json_bindings)
        {
            type = define_from_json_bindings<T>(type);
            type = define_to_json_bindings<T>(type);
        }

        if constexpr (config.generate_binary_bindings)
        {
            type = define_from_binary_bindings<T>(type);
            type = define_to_binary_bindings<T>(type);
        }

        if constexpr (config.generate_optional_reflection)
        {
            optional_engine_meta_type<T>(false);
        }

        if constexpr (config.generate_operator_wrappers)
        {
            type = define_unary_meta_operator<has_function_operator_plus,        T>(type, "+operator"_hs);
            type = define_unary_meta_operator<has_function_operator_minus,       T>(type, "-operator"_hs);
            type = define_unary_meta_operator<has_function_operator_bitwise_not, T>(type, "~operator"_hs);

            type = define_meta_operator<has_function_operator_multiply, has_method_operator_multiply, T>(type, "operator*"_hs);
            type = define_meta_operator<has_function_operator_divide,   has_method_operator_divide,   T>(type, "operator/"_hs);
            type = define_meta_operator<has_function_operator_plus,     has_method_operator_plus,     T>(type, "operator+"_hs);
            type = define_meta_operator<has_function_operator_minus,    has_method_operator_minus,    T>(type, "operator-"_hs);

            type = define_meta_operator<has_function_operator_bitwise_shift_left,  has_method_operator_bitwise_shift_left,  T>(type, "operator<<"_hs);
            type = define_meta_operator<has_function_operator_bitwise_shift_right, has_method_operator_bitwise_shift_right, T>(type, "operator>>"_hs);

            type = define_equality_operators<T>(type);

            type = define_meta_operator<has_function_operator_bitwise_and, has_method_operator_bitwise_and, T>(type, "operator&"_hs);
            type = define_meta_operator<has_function_operator_bitwise_xor, has_method_operator_bitwise_xor, T>(type, "operator^"_hs);
            type = define_meta_operator<has_function_operator_bitwise_or,  has_method_operator_bitwise_or,  T>(type, "operator|"_hs);

            type = define_meta_operator_method<has_method_operator_less_than,             T, bool, const T&, true>(type, "operator<"_hs); // true
            type = define_meta_operator_method<has_method_operator_less_than_or_equal,    T, bool, const T&, true>(type, "operator<="_hs); // true
            type = define_meta_operator_method<has_method_operator_greater_than,          T, bool, const T&, true>(type, "operator>"_hs); // true
            type = define_meta_operator_method<has_method_operator_greater_than_or_equal, T, bool, const T&, true>(type, "operator>="_hs); // true

            type = define_boolean_operators<T>(type);

            //type = define_meta_operator_method<has_method_operator_assign, T, T&, const T&, false>(type, "operator="_hs);
            //type = define_meta_operator_method<has_method_operator_assign, T, T&, MetaAny, false>(type, "operator="_hs);

            type = define_meta_operator_method<has_method_operator_add_assign,      T, T&, const T&, false>(type, "operator+="_hs);
            type = define_meta_operator_method<has_method_operator_subtract_assign, T, T&, const T&, false>(type, "operator-="_hs);
            type = define_meta_operator_method<has_method_operator_multiply_assign, T, T&, const T&, false>(type, "operator*="_hs);
            type = define_meta_operator_method<has_method_operator_divide_assign,   T, T&, const T&, false>(type, "operator/="_hs);
            type = define_meta_operator_method<has_method_operator_modulus_assign,  T, T&, const T&, false>(type, "operator%="_hs);

            type = define_meta_operator_method<has_method_operator_bitwise_shift_left_assign,  T, T&, const T&, false>(type, "operator<<="_hs);
            type = define_meta_operator_method<has_method_operator_bitwise_shift_right_assign, T, T&, const T&, false>(type, "operator>>="_hs);
            type = define_meta_operator_method<has_method_operator_bitwise_and_assign,         T, T&, const T&, false>(type, "operator&="_hs);
            type = define_meta_operator_method<has_method_operator_bitwise_xor_assign,         T, T&, const T&, false>(type, "operator^="_hs);
            type = define_meta_operator_method<has_method_operator_bitwise_or_assign,          T, T&, const T&, false>(type, "operator|="_hs);

            type = define_meta_operator_method<has_method_operator_subscript, T, MetaAny, const MetaAny&, true>(type, "operator[]"_hs);

            // TODO: Determine if it makes sense to add support for custom dereference operators.
        }

        if constexpr (config.generate_indirect_getters)
        {
            define_indirect_getters<T, MetaAny>(type);
            define_indirect_getters<T, Entity>(type);
        }

        if constexpr (config.generate_indirect_setters)
        {
            define_indirect_setters<T, MetaAny>(type);
            define_indirect_setters<T, T&>(type);
        }

        if constexpr ((config.capture_standard_data_members) && (!std::is_empty_v<T>))
        {
            // `entity` data member:
            if constexpr (has_method_entity<T, Entity>::value) // std::decay_t<T>
            {
                type = type.data<nullptr, &T::entity>("entity"_hs);
            }
            else if constexpr (has_method_get_entity<T, Entity>::value) // std::decay_t<T>
            {
                type = type.data<nullptr, &T::get_entity>("entity"_hs);
            }
            else if constexpr (has_field_entity<T>::value) // std::decay_t<T>
            {
                type = type.data<&T::entity>("entity"_hs);
            }

            // `player` data member:
            if constexpr (has_method_player<T, Entity>::value) // std::decay_t<T>
            {
                type = type.data<nullptr, &T::player>("player"_hs);
            }
            else if constexpr (has_method_get_player<T, Entity>::value) // std::decay_t<T>
            {
                type = type.data<nullptr, &T::get_player>("player"_hs);
            }
            else if constexpr (has_field_player<T>::value) // std::decay_t<T>
            {
                type = type.data<&T::player>("player"_hs);
            }

            // `player_index` data member:
            if constexpr (has_method_player_index<T, PlayerIndex>::value) // std::decay_t<T>
            {
                type = type.data<nullptr, &T::player_index>("player_index"_hs);
            }
            else if constexpr (has_method_get_player_index<T, PlayerIndex>::value) // std::decay_t<T>
            {
                type = type.data<nullptr, &T::get_player_index>("player_index"_hs);
            }
            else if constexpr (has_field_player_index<T>::value) // std::decay_t<T>
            {
                type = type.data<&T::player_index>("player_index"_hs);
            }

            // `service` data member:
            if constexpr (has_method_service<T, Service*>::value)
            {
                type = type.data<nullptr, &T::service>("service"_hs);
            }
            else if constexpr (has_method_service<T, Service&>::value)
            {
                type = type.data<nullptr, &T::service>("service"_hs);
            }
            else if constexpr (has_method_get_service<T, Service*>::value)
            {
                type = type.data<nullptr, &T::get_service>("service"_hs);
            }
            else if constexpr (has_method_get_service<T, Service&>::value)
            {
                type = type.data<nullptr, &T::get_service>("service"_hs);
            }
            // NOTE: Requires full definition of `Service` type.
            else if constexpr (has_field_service<T>::value)
            {
                type = type.data<&T::service>("service"_hs);
            }
        }

        return type;
    }

    template <typename T, MetaTypeReflectionConfig config={}>
    auto custom_meta_type(MetaTypeID type_id, bool sync_context=true)
    {
        if (sync_context)
        {
            // Ensure that we're using the correct context.
            sync_reflection_context();
        }

        auto type = entt::meta<T>()
            .type(type_id)
        ;

        return define_standard_meta_type<T, config>(type, false);
    }

    // Associates a stripped version of the type's name to its reflection metadata.
    // Allows use of `T` without specifying `engine::T` in contexts where `T` is named dynamically. (e.g. JSON I/O)
    // 
    // By default, `entt` uses a fully qualified name, along with a "struct" or "class" prefix, etc.
    // This allows you to simply refer to the type by its namespace-local name.
    template <typename T, MetaTypeReflectionConfig config={}>
    auto engine_meta_type(bool sync_context=true)
    {
        constexpr auto type_name = short_name<T>();
        constexpr auto type_id = hash(type_name);

        auto type = custom_meta_type<T, config>(type_id, sync_context);

        if constexpr (type_name.ends_with("Component"))
        {
            type = type
                .prop("component"_hs)
            ;
        }

        return type;
    }

    template <typename T> // MetaTypeReflectionConfig config={}
    auto static_engine_meta_type(bool sync_context=true)
    {
        if (sync_context)
        {
            // Ensure that we're using the correct context.
            sync_reflection_context();
        }

        auto type = entt::meta<T>()
            .type(short_name_hash<T>())
        ;

        return type;
    }

    template <typename T>
    auto engine_service_type(bool sync_context=true)
    {
        return static_engine_meta_type<T>(sync_context)
            .prop("service"_hs)
        ;
    }

    template <typename T>
    auto engine_system_type(bool sync_context=true)
    {
        return static_engine_meta_type<T>(sync_context)
            .prop("system"_hs)
        ;
    }

    template <typename T>
    auto engine_command_type(bool sync_context=true)
    {
        return engine_meta_type
        <
            T,
            
            MetaTypeReflectionConfig
            {
                .capture_standard_data_members = false
            }
        >(sync_context)
            .base<Command>()
            //.prop("command"_hs)
        ;
    }

    // TODO: Look into this implementation again.
    // (Probably not very efficient to use properties for this)
    template <typename EnumType, bool generate_optional_reflection=true>
    auto reflect_enum(MetaTypeID type_id, bool values_as_properties=false)
    {
        auto type = custom_meta_type<EnumType>(type_id)
            .func<&string_to_enum_value<EnumType>>("string_to_value"_hs)
            .ctor<&string_to_enum_value<EnumType>>()
            .conv<&enum_value_to_string<EnumType>>()
        ;

        if (values_as_properties)
        {
            magic_enum::enum_for_each<EnumType>
            (
                [](EnumType enum_value)
                {
                    const auto enum_name = magic_enum::enum_name<EnumType>(enum_value);
                    const auto enum_value_id = hash(enum_name);

                    entt::meta<EnumType>().prop(enum_value_id, enum_value);
                }
            );
        }

        if constexpr (generate_optional_reflection)
        {
            optional_engine_meta_type<EnumType>(false);
        }

        return type;
    }

    // NOTE: This is called automatically via `reflect` when `T` is an enumeration type.
    template <typename EnumType>
    void reflect_enum(bool values_as_properties=false)
    {
        reflect_enum<EnumType>(short_name_hash<EnumType>(), values_as_properties);
    }
}