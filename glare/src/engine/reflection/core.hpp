#pragma once

#include "context.hpp"
#include "indirection.hpp"
#include "operators.hpp"
#include "optional.hpp"
#include "history.hpp"
#include "binary_bindings.hpp"


#include "service_extensions.hpp"
#include "component_extensions.hpp"
#include "meta_type_reflection_config.hpp"
#include "json_bindings.hpp"

#include <engine/meta/meta.hpp>
#include <engine/meta/traits.hpp>
#include <engine/meta/short_name.hpp>
#include <engine/meta/meta_event_listener.hpp>

#include <type_traits>

// Declares a meta-type for a type with a single field.
#define REFLECT_SINGLE_FIELD_TYPE(type_name, field_name)                \
    engine::engine_meta_type<type_name>()                               \
		.data<&type_name::field_name>(entt::hashed_string(#field_name)) \
		.ctor<decltype(type_name::field_name)>();

// Declares a meta-type for a type, derived from another type, with a single field.
#define REFLECT_SINGLE_FIELD_DERIVED_TYPE(type_name, base_type_name, field_name) \
    engine::engine_meta_type<type_name>()                                        \
        .base<base_type_name>()                                                  \
		.data<&type_name::field_name>(entt::hashed_string(#field_name))

#define REFLECT_SINGLE_FIELD_COMPONENT REFLECT_SINGLE_FIELD_TYPE

namespace engine
{
    using namespace engine::literals;

    class Service;

    struct MetaEvaluationContext;

    struct MetaParsingInstructions;
    struct MetaTypeDescriptorFlags;

	namespace impl
    {
        template <typename T, MetaTypeReflectionConfig config={}>
        auto define_standard_meta_type(auto type)
        {
            type = type
                .template func<&impl::from_meta<T>>("from_meta"_hs)
                .template func<&short_name<T>>("get_type_name"_hs)
                .template func<&impl::trigger_event_from_meta_any<T>>("trigger_event_from_meta_any"_hs)
                .template func<&try_get_underlying_type<T>>("try_get_underlying_type"_hs)

                // Disabled for now. (`std::is_polymorphic` doesn't seem reliable enough for use here)
                //.template func<&impl::from_void_ptr<T>>("dynamic_cast"_hs)
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
                        engine_history_component_type<T>(false);

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
                engine_optional_type<T>(false);
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
                    type = type.data<nullptr, has_method_player<T, Entity>::ptr<true>>("player"_hs);
                }
                else if constexpr (has_method_get_player<T, Entity>::value) // std::decay_t<T>
                {
                    type = type.data<nullptr, has_method_get_player<T, Entity>::ptr<true>>("player"_hs);
                }
                else if constexpr (has_field_player<T>::value) // std::decay_t<T>
                {
                    type = type.data<&T::player>("player"_hs);
                }

                // `player_index` data member:
                if constexpr (has_method_player_index<T, PlayerIndex>::value) // std::decay_t<T>
                {
                    type = type.data<nullptr, has_method_player_index<T, PlayerIndex>::ptr<true>>("player_index"_hs);
                }
                else if constexpr (has_method_get_player_index<T, PlayerIndex>::value) // std::decay_t<T>
                {
                    type = type.data<nullptr, has_method_get_player_index<T, PlayerIndex>::ptr<true>>("player_index"_hs);
                }
                else if constexpr (has_field_player_index<T>::value) // std::decay_t<T>
                {
                    type = type.data<&T::player_index>("player_index"_hs);
                }

                // `service` data member:
                if constexpr (has_method_service<T, Service*>::value)
                {
                    type = type.data<nullptr, has_method_service<T, Service*>::ptr<false>>("service"_hs);
                }
                else if constexpr (has_method_service<T, Service&>::value)
                {
                    type = type.data<nullptr, has_method_service<T, Service&>::ptr<false>>("service"_hs);
                }
                else if constexpr (has_method_get_service<T, Service*>::value)
                {
                    type = type.data<nullptr, has_method_get_service<T, Service*>::ptr<false>>("service"_hs);
                }
                else if constexpr (has_method_get_service<T, Service&>::value)
                {
                    type = type.data<nullptr, has_method_get_service<T, Service&>::ptr<false>>("service"_hs);
                }
                // NOTE: Requires full definition of `Service` type.
                else if constexpr (has_field_service<T>::value)
                {
                    type = type.data<&T::service>("service"_hs);
                }
                else if constexpr (has_method_service<T, const Service*>::value)
                {
                    type = type.data<nullptr, has_method_service<T, const Service*>::ptr<true>>("service"_hs);
                }
                else if constexpr (has_method_service<T, const Service&>::value)
                {
                    type = type.data<nullptr, has_method_service<T, const Service&>::ptr<true>>("service"_hs);
                }
                else if constexpr (has_method_get_service<T, const Service*>::value)
                {
                    type = type.data<nullptr, has_method_get_service<T, const Service*>::ptr<true>>("service"_hs);
                }
                else if constexpr (has_method_get_service<T, const Service&>::value)
                {
                    type = type.data<nullptr, has_method_get_service<T, const Service&>::ptr<true>>("service"_hs);
                }
            }

            return type;
        }
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

        return impl::define_standard_meta_type<T, config>(type);
    }

    constexpr auto custom_meta_type_apply_name_traits(auto type, auto&& type_name)
    {
        if (type_name.ends_with("Component"))
        {
            type = type
                .prop("component"_hs)
            ;
        }

        return type;
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
        constexpr auto type_id   = hash(type_name);

        auto type = custom_meta_type<T, config>(type_id, sync_context);

        type = custom_meta_type_apply_name_traits(type, type_name);

        return type;
    }
}