#pragma once

// Internal header for `engine` module.
// 
// This header defines general-purpose reflection facilities for systems, components, etc. using `entt`'s meta-type system.
// For consuming or instantiating reflection data, it is recommended that you directly use the `meta` header instead.

#include "types.hpp"

#include "meta/meta.hpp"
#include "meta/traits.hpp"
#include "meta/meta_type_descriptor.hpp"
#include "meta/meta_event_listener.hpp"

#include "service.hpp"
#include "command.hpp"

#include <util/reflection.hpp>

#include <string_view>
#include <type_traits>
#include <optional>
#include <stdexcept>

// Debugging related:
#include <util/log.hpp>

// Declares a meta-type for a component with a single field.
#define REFLECT_SINGLE_FIELD_TYPE(type_name, field_name)                \
    engine::engine_meta_type<type_name>()                               \
		.data<&type_name::field_name>(entt::hashed_string(#field_name)) \
		.ctor<decltype(type_name::field_name)>();

#define GENERATE_SINGLE_FIELD_TYPE_REFLECTION(type_name, field_name) \
    template <>                                                      \
    inline void reflect<type_name>()                                 \
    {                                                                \
        REFLECT_SINGLE_FIELD_COMPONENT(type_name, field_name);       \
    }

#define GENERATE_SINGLE_FIELD_COMPONENT_REFLECTION GENERATE_SINGLE_FIELD_TYPE_REFLECTION
#define REFLECT_SINGLE_FIELD_COMPONENT REFLECT_SINGLE_FIELD_TYPE

// Generates an empty `reflect` function for the specified `engine` type.
#define GENERATE_EMPTY_TYPE_REFLECTION(type) \
    template <>                              \
    inline void reflect<type>()              \
    {                                        \
        engine::engine_meta_type<type>();    \
    }

// Generates an empty `reflect` function for the specified `engine` type,
// where `type` is derived from `base_type`.
#define GENERATE_EMPTY_DERIVED_TYPE_REFLECTION(type, base_type) \
    template <>                                                 \
    inline void reflect<type>()                                 \
    {                                                           \
        engine::engine_meta_type<type>()                        \
            .base<base_type>()                                  \
        ;                                                       \
    }

namespace engine
{
	using namespace entt::literals;

    class Service;

    // NOTE: In the default case of `T=void`, the overridden version of this template is used.
    // TODO: Look into best way to handle multiple calls to reflect. (This is currently only managed in `reflect_all`)
    template <typename T=void>
    inline void reflect()
    {
        if constexpr (std::is_enum_v<T>)
        {
            reflect_enum<T>();
        }
        else if constexpr (has_function_reflect<T, void()>)
        {
            T::reflect();
        }
        else if constexpr (has_function_reflect<T, entt::meta_factory<T>()>)
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

    // Retrieves a pointer to `T` from `value`.
    // 
    // If `value` holds a `T` instance, this will return a pointer as expected.
    // If `value` is empty, or if the value is of a different type, this will return `nullptr`.
    template <typename T>
    T* from_meta(entt::meta_any& value)
    {
        if (!value)
        {
            return nullptr;
        }

        if
        (
            (value.type().id() == short_name_hash<T>().value())
            ||
            (value.type().id() == entt::type_hash<T>::value())
        )
        {
            return reinterpret_cast<T*>(value.data());
        }

        return nullptr;
    }

    template <typename T>
    T& emplace_meta_component(Registry& registry, Entity entity, entt::meta_any value)
    {
        if (auto raw_value = from_meta<T>(value))
        {
            return registry.emplace_or_replace<T>(entity, std::move(*raw_value)); // *raw_value
        }
        else
        {
            throw std::exception("Invalid value specified; unable to attach component.");
        }
    }

    template <typename T>
    bool remove_component(Registry& registry, Entity entity)
    {
        // Check if we currently have an instance of `T` attached to `entity`:
        if (!registry.try_get<T>(entity))
        {
            return false;
        }

        registry.erase<T>(entity);

        return true;
    }

    template <typename T>
    T& get_or_emplace_component(Registry& registry, Entity entity, entt::meta_any value)
    {
        if (auto raw_value = from_meta<T>(value))
        {
            return registry.get_or_emplace<T>(entity, std::move(*raw_value)); // *raw_value
        }
        else
        {
            throw std::exception("Invalid value specified; unable to attach component.");
        }
    }

    template <typename T>
    T* get_component(Registry& registry, Entity entity)
    {
        return registry.try_get<T>(entity);
    }

    template <typename T>
    bool has_component(Registry& registry, Entity entity)
    {
        return static_cast<bool>(get_component<T>(registry, entity));
    }

    // Moves component `T` from `entity` into an `entt::meta_any` instance,
    // then removes the component-type from `entity`.
    template <typename T>
    entt::meta_any store_meta_component(Registry& registry, Entity entity)
    {
        if constexpr (true) // (std::is_move_assignable_v<T>) // (std::is_move_constructible_v<T>)
        {
            auto* instance = get_component<T>(registry, entity);

            if (!instance)
            {
                return {};
            }

            entt::meta_any output = std::move(*instance);

            registry.erase<T>(entity);

            // Alternate (slower):
            // remove_component<T>(registry, entity);

            return output;
        }
        else
        {
            return {};
        }
    }

    // Generates a copy of component `T` from `entity` into an `entt::meta_any` instance.
    // the original component instance remains attached to `entity`.
    template <typename T>
    entt::meta_any copy_meta_component(Registry& registry, Entity entity)
    {
        if constexpr (std::is_copy_constructible_v<T>)
        {
            auto* instance = get_component<T>(registry, entity);

            if (!instance)
            {
                return {};
            }

            return T(*instance);
        }
        else
        {
            return {};
        }
    }

    template <typename T>
    std::size_t patch_meta_component(Registry& registry, Entity entity, const MetaTypeDescriptor& descriptor, std::size_t field_count, std::size_t offset)
    {
        assert
        (
            (descriptor.type.id() == short_name_hash<T>().value())
            ||
            (descriptor.type.id() == entt::type_hash<T>::value())
        );

        // Ensure `T` is currently a component of `entity`.
        if (!registry.try_get<T>(entity))
        {
            return 0;
        }

        std::size_t count = 0;

        registry.patch<T>(entity, [&descriptor, &field_count, &offset, &count](T& instance)
        {
            auto any_wrapper = entt::forward_as_meta(instance);

            count = descriptor.apply_fields(any_wrapper, field_count, offset);
        });

        return count;
    }

    template <typename EventType>
    void trigger_event_from_meta_any(Service& service, entt::meta_any event_instance)
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

    // Associates a stripped version of the type's name to its reflection metadata.
    // Allows use of `T` without specifying `engine::T` in contexts where `T` is named dynamically. (e.g. JSON I/O)
    // 
    // By default, `entt` uses a fully qualified name, along with a "struct" or "class" prefix, etc.
    // This allows you to simply refer to the type by its namespace-local name.
    template <typename T>
    auto engine_meta_type()
    {
        auto type = entt::meta<T>()
            .type(short_name_hash<T>())

            .template func<from_meta<T>>("from_meta"_hs)
            .template func<has_component<T>>("has_component"_hs)
            .template func<get_component<T>>("get_component"_hs)
            .template func<&emplace_meta_component<T>, entt::as_ref_t>("emplace_meta_component"_hs)
            .template func<&store_meta_component<T>>("store_meta_component"_hs)
            .template func<&copy_meta_component<T>>("copy_meta_component"_hs)
            .template func<&remove_component<T>>("remove_component"_hs)
            .template func<&get_or_emplace_component<T>, entt::as_ref_t>("get_or_emplace_component"_hs)
            .template func<&patch_meta_component<T>>("patch_meta_component"_hs)
            .template func<&MetaEventListener::connect<T>>("connect_meta_event"_hs)
            .template func<&MetaEventListener::disconnect<T>>("disconnect_meta_event"_hs)
            .template func<&trigger_event_from_meta_any<T>>("trigger_event_from_meta_any"_hs);
        ;

        // `entity` data member:
        if constexpr (has_method_entity<T, Entity()>::value) // std::decay_t<T>
        {
            type = type.data<nullptr, &T::entity>("entity"_hs);
        }
        else if constexpr (has_method_get_entity<T, Entity()>::value) // std::decay_t<T>
        {
            type = type.data<nullptr, &T::get_entity>("entity"_hs);
        }
        else if constexpr (has_field_entity<T>::value) // std::decay_t<T>
        {
            type = type.data<&T::entity>("entity"_hs);
        }

        // `player` data member:
        if constexpr (has_method_player<T, Entity()>::value) // std::decay_t<T>
        {
            type = type.data<nullptr, &T::player>("player"_hs);
        }
        else if constexpr (has_method_get_player<T, Entity()>::value) // std::decay_t<T>
        {
            type = type.data<nullptr, &T::get_player>("player"_hs);
        }
        else if constexpr (has_field_player<T>::value) // std::decay_t<T>
        {
            type = type.data<&T::player>("player"_hs);
        }

        // `player_index` data member:
        if constexpr (has_method_player_index<T, PlayerIndex()>::value) // std::decay_t<T>
        {
            type = type.data<nullptr, &T::player_index>("player_index"_hs);
        }
        else if constexpr (has_method_get_player_index<T, PlayerIndex()>::value) // std::decay_t<T>
        {
            type = type.data<nullptr, &T::get_player_index>("player_index"_hs);
        }
        else if constexpr (has_field_player_index<T>::value) // std::decay_t<T>
        {
            type = type.data<&T::player_index>("player_index"_hs);
        }

        // `service` data member:
        if constexpr (has_method_service<T, Service*()>::value)
        {
            type = type.data<nullptr, &T::service>("service"_hs);
        }
        else if constexpr (has_method_service<T, Service&()>::value)
        {
            type = type.data<nullptr, &T::service>("service"_hs);
        }
        else if constexpr (has_method_get_service<T, Service*()>::value)
        {
            type = type.data<nullptr, &T::get_service>("service"_hs);
        }
        else if constexpr (has_method_get_service<T, Service&()>::value)
        {
            type = type.data<nullptr, &T::get_service>("service"_hs);
        }
        // NOTE: Requires full definition of `Service` type.
        else if constexpr (has_field_service<T>::value)
        {
            type = type.data<&T::service>("service"_hs);
        }

        return type;
    }

    template <typename T>
    auto engine_command_type()
    {
        return engine_meta_type<T>()
            .base<Command>()
        ;
    }

    template <>
    inline void reflect<Command>()
    {
        engine_meta_type<Command>()
            .data<&Command::source>("source"_hs)
            .data<&Command::target>("target"_hs)
            //.ctor<decltype(Command::source), decltype(Command::target)>()
        ;
    }

    template <typename EnumType>
    EnumType string_to_enum_value(std::string_view enum_short_str)
    {
        auto result = magic_enum::enum_cast<EnumType>(enum_short_str);

        if (!result)
        {
            throw std::invalid_argument("Invalid enum value specified.");
        }

        return *result;
    }

    // NOTE: This is called automatically via `reflect` when `T` is an enumeration type.
    //
    // TODO: Look into this implementation again.
    // (Probably not very efficient to use properties for this)
    template <typename EnumType>
    void reflect_enum(bool values_as_properties=false)
    {
        auto meta_obj = engine_meta_type<EnumType>()
            .template func<&string_to_enum_value<EnumType>>("string_to_value"_hs)
        ;
		
        if (values_as_properties)
        {
            magic_enum::enum_for_each<EnumType>([](EnumType enum_value)
            {
                const auto enum_name = magic_enum::enum_name<EnumType>(enum_value);
                const auto enum_value_id = hash(enum_name);

                entt::meta<EnumType>().prop(enum_value_id, enum_value);
            });
        }
    }
}