#pragma once

// This is an internal header for `engine` module and should only be used by reflection implementation files.
// 
// This header defines general-purpose reflection facilities for systems, components, etc. using EnTT's meta-type system.
// For consuming or instantiating reflection data, it is recommended that you directly use the `meta` header instead.

#include "types.hpp"

#include "meta/meta.hpp"
#include "meta/serial.hpp"
#include "meta/traits.hpp"
#include "meta/meta_type_descriptor.hpp"
#include "meta/meta_event_listener.hpp"
#include "meta/meta_evaluation_context.hpp"

#include "service.hpp"
#include "command.hpp"

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

    //struct MetaEvaluationContext;

    struct MetaParsingInstructions;
    struct MetaTypeDescriptorFlags;

    struct MetaTypeReflectionConfig
    {
        bool capture_standard_data_members : 1 = true;
        bool generate_optional_reflection  : 1 = true;
        bool generate_operator_wrappers    : 1 = true;
        bool generate_indirect_getters     : 1 = true;
        bool generate_indirect_setters     : 1 = true;
        bool generate_json_constructor     : 1 = true;
    };

    template
    <
        typename T, typename trait,
        bool is_const=false, bool is_noexcept=false,
        typename policy=entt::as_is_t
    >
    bool reflect_function(auto& type, auto function_id)
    {
        if constexpr (trait::value)
        {
            if constexpr (trait::template ptr<is_const, is_noexcept>)
            {
                type = type.template func
                <
                    trait::template ptr<is_const, is_noexcept>,
                    policy
                >(function_id);

                return true;
            }
            else
            {
                return false;
            }
        }
        else
        {
            return false;
        }
    }

    template <typename T, bool is_const, bool is_noexcept, typename policy, typename ...traits>
    auto& reflect_any_function(auto& type, auto function_id) // constexpr
    {
        (reflect_function<T, traits, is_const, is_noexcept, policy>(type, function_id) || ...);

        return type;
    }

    template <typename T, bool is_const, bool is_noexcept, typename policy, typename ...traits>
    auto& reflect_any_function(auto& type, auto function_id, bool& result_out) // constexpr
    {
        if ((reflect_function<T, traits, is_const, is_noexcept, policy>(type, function_id) || ...))
        {
            result_out = true;
        }

        return type;
    }

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
    bool operator_bool_impl(const T& value)
    {
        //return static_cast<bool>(value);

        ///*
        if (value)
        {
            return true;
        }

        return false;
        //*/
    }

    template <typename T>
    bool operator_logical_not_impl(const T& value)
    {
        return !operator_bool_impl<T>(value);
    }

    // Implements the inequality operator using the equality operator.
    template <typename TypeA, typename TypeB>
    bool inequality_operator_fallback_impl(const TypeA& a, const TypeB& b)
    {
        return !(a == b);
    }

    // Implements the equality operator using the inequality operator.
    template <typename TypeA, typename TypeB>
    bool equality_operator_fallback_impl(const TypeA& a, const TypeB& b)
    {
        return !(a != b);
    }

    // Retrieves a pointer to `T` from `value`.
    // 
    // If `value` holds a `T` instance, this will return a pointer as expected.
    // If `value` is empty, or if the value is of a different type, this will return `nullptr`.
    template <typename T>
    T* from_meta(MetaAny& value)
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
	T from_json(const util::json& data)
    {
        return engine::load<T>(data);
    }

    // NOTE: Named differently from `from_json` due to ICE on latest MSVC compiler.
    template <typename T>
	T from_json_with_instructions
	(
		const util::json& data,
        const MetaParsingInstructions& parse_instructions
	)
    {
        return engine::load<T>(data, parse_instructions);
    }

    // NOTE: Named differently from `from_json` due to ICE on latest MSVC compiler.
    template <typename T>
	T from_json_with_instructions_and_descriptor_flags
	(
		const util::json& data,
		const MetaParsingInstructions& parse_instructions,
		const MetaTypeDescriptorFlags& descriptor_flags
	)
    {
        return engine::load<T>(data, parse_instructions, descriptor_flags);
    }

    template <typename T>
    T& emplace_meta_component(Registry& registry, Entity entity, MetaAny& value) // MetaAny&&
    {
        if (auto raw_value = from_meta<T>(value))
        {
            if constexpr (std::is_move_constructible_v<T>)
            {
                if constexpr (std::is_move_assignable_v<T>)
                {
                    return registry.emplace_or_replace<T>(entity, std::move(*raw_value)); // *raw_value
                }
                else if constexpr (std::is_copy_assignable_v<T>)
                {
                    auto active_instance = registry.try_get<T>(entity);

                    if (active_instance)
                    {
                        *active_instance = *raw_value;

                        // Mark this component as patched.
                        registry.patch<T>(entity);

                        return *active_instance;
                    }
                    else
                    {
                        return registry.emplace<T>(entity, std::move(*raw_value));
                    }

                    //return registry.emplace_or_replace<T>(entity, *raw_value); // std::move(*raw_value)
                }
                else
                {
                    if (registry.try_get<T>(entity))
                    {
                        registry.remove<T>(entity);
                    }

                    return registry.emplace<T>(entity, std::move(*raw_value)); // *raw_value
                }
            }
            else if constexpr (std::is_copy_constructible_v<T>)
            {
                if constexpr (std::is_copy_assignable_v<T>)
                {
                    return registry.emplace_or_replace<T>(entity, *raw_value);
                }
                else if constexpr (std::is_move_assignable_v<T>)
                {
                    auto active_instance = registry.try_get<T>(entity);

                    if (active_instance)
                    {
                        *active_instance = std::move(*raw_value);

                        // Mark this component as patched.
                        registry.patch<T>(entity);

                        return *active_instance;
                    }
                    else
                    {
                        return registry.emplace<T>(entity, *raw_value);
                    }

                    //return registry.emplace_or_replace<T>(entity, std::move(*raw_value));
                }
                else
                {
                    if (registry.try_get<T>(entity))
                    {
                        registry.remove<T>(entity);
                    }

                    return registry.emplace<T>(entity, *raw_value);
                }
            }
        }

        throw std::exception("Invalid value specified; unable to attach component.");
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
    T& get_or_emplace_component(Registry& registry, Entity entity, MetaAny& value)
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

    // Moves component `T` from `entity` into a `MetaAny` instance,
    // then removes the component-type from `entity`.
    template <typename T>
    MetaAny store_meta_component(Registry& registry, Entity entity)
    {
        if constexpr (true) // (std::is_move_assignable_v<T>) // (std::is_move_constructible_v<T>)
        {
            auto* instance = get_component<T>(registry, entity);

            if (!instance)
            {
                return {};
            }

            MetaAny output = std::move(*instance);

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

    // Generates a copy of component `T` from `entity` into a `MetaAny` instance.
    // the original component instance remains attached to `entity`.
    template <typename T>
    MetaAny copy_meta_component(Registry& registry, Entity entity)
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

    // Applies a patch to the component `T` attached to `target`, from the context of `source`.
    // If `source` is not specified, `target` will act as the `source` as well.
    template <typename T>
    std::size_t indirect_patch_meta_component(Registry& registry, Entity target, const MetaTypeDescriptor& descriptor, std::size_t field_count, std::size_t offset, Entity source=null, const MetaEvaluationContext* opt_evaluation_context=nullptr, const MetaAny& opt_context_value={})
    {
        assert
        (
            (descriptor.get_type_id() == short_name_hash<T>().value())
            ||
            (descriptor.get_type_id() == entt::type_hash<T>::value())
        );

        // Ensure `T` is currently a component of `target`.
        if (!registry.try_get<T>(target))
        {
            return 0;
        }

        std::size_t count = 0;

        if (source == null)
        {
            source = target;
        }

        registry.patch<T>(target, [&registry, source, &descriptor, &field_count, &offset, &opt_evaluation_context, &opt_context_value, &count](T& instance)
        {
            auto any_wrapper = entt::forward_as_meta(instance);

            if (opt_context_value)
            {
                if (opt_evaluation_context)
                {
                    count = descriptor.apply_fields(any_wrapper, opt_context_value, registry, source, *opt_evaluation_context, field_count, offset);
                }
                else
                {
                    count = descriptor.apply_fields(any_wrapper, opt_context_value, registry, source, field_count, offset);
                }
            }
            else
            {
                if (opt_evaluation_context)
                {
                    count = descriptor.apply_fields(any_wrapper, registry, source, *opt_evaluation_context, field_count, offset);
                }
                else
                {
                    count = descriptor.apply_fields(any_wrapper, registry, source, field_count, offset);
                }
            }
        });

        return count;
    }

    // Applies a patch to the component `T` attached to `target`, from the context of `source`.
    // If `source` is not specified, `target` will act as the `source` as well.
    template <typename T>
    T& direct_patch_meta_component(Registry& registry, Entity entity, MetaAny& value, bool perform_data_member_sweep=false)
    {
        auto active_component_instance = registry.try_get<T>(entity);

        if (!active_component_instance)
        {
            return emplace_meta_component<T>(registry, entity, value); // std::move(value);
        }

        if (auto raw_value = from_meta<T>(value))
        {
            if constexpr (std::is_move_assignable_v<T>)
            {
                if (!perform_data_member_sweep)
                {
                    *active_component_instance = std::move(*raw_value); // *raw_value;
                }
            }
            else if constexpr (std::is_copy_assignable_v<T>)
            {
                if (!perform_data_member_sweep)
                {
                    *active_component_instance = *raw_value;
                }
            }
            else
            {
                if (!perform_data_member_sweep)
                {
                    throw std::exception("Patch attempted with unsupported component; unable to find assignment operator.");
                }
            }

            if (perform_data_member_sweep)
            {
                auto component_type = value.type();

                auto active_component_ref = entt::forward_as_meta(active_component_instance);
                auto patch_component_ref  = entt::forward_as_meta(raw_value);

                for (auto data_member_entry : component_type.data())
                {
                    auto& data_member = data_member_entry.second;

                    if (data_member.is_const())
                    {
                        continue;
                    }

                    // TODO: Validate that this works as expected. (i.e. `MetaAny` inputs to `set`)
                    data_member.set(active_component_ref, data_member.get(patch_component_ref));
                }
            }

            // Mark the component as patched.
            registry.patch<T>(entity);

            return *active_component_instance;
        }
        else
        {
            throw std::exception("Invalid value specified; unable to patch component.");
        }
    }

    // Notifies listeners that a component of type `T` has been patched.
    template <typename T>
    void mark_component_as_patched(Registry& registry, Entity entity)
    {
        registry.patch<T>(entity);
    }

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

    template <typename T>
    T& from_optional(std::optional<T>& opt)
    {
        assert(opt);

        return *opt;
    }

    template <typename T>
    MetaTypeID type_id_from_optional()
    {
        //return short_name_hash<T>();

        auto type = resolve<T>();

        if (!type)
        {
            return MetaTypeID(0);
        }

        return type.id();
    }

    template <typename T>
    MetaType type_from_optional()
    {
        //return resolve(type_id_from_optional<T>());

        return resolve<T>();
    }

    // Retrieves a handle to the shared entt meta-context.
    entt::locator<entt::meta_ctx>::node_type get_shared_reflection_handle();
    
    // NOTE: This implementation is inline so that it is local to this module.
    // In contrast, the `get_shared_reflection_handle` function is unique between
    // modules (i.e. DLLs) since it's explicitly defined in a translation unit.
    inline void set_local_reflection_handle(const entt::locator<entt::meta_ctx>::node_type& handle)
    {
        entt::locator<entt::meta_ctx>::reset(handle);
    }

    // Synchronizes the reflection context across boundaries. (used for DLL interop)
    inline bool sync_reflection_context()
    {
        static bool is_synchronized = false;

        if (is_synchronized)
        {
            return true;
        }

        const auto& handle = get_shared_reflection_handle();
        set_local_reflection_handle(handle);

        is_synchronized = true;

        return is_synchronized;
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
    auto optional_engine_meta_type()
    {
        auto opt_type = entt::meta<std::optional<T>>()
            .type(optional_short_name_hash<T>())
            //.template func<&from_optional<T>>("from_optional"_hs)
            .template func<&type_id_from_optional<T>>("type_id_from_optional"_hs)
            .template func<&type_from_optional<T>>("type_from_optional"_hs)

            //.base<T>()
            //.conv<T>()
        ;

        if constexpr (std::is_copy_constructible_v<T>)
        {
            opt_type = opt_type.ctor<T>(); // const T&
        }

        /*
        if constexpr (std::is_move_constructible_v<T>)
        {
            opt_type = opt_type.ctor<T&&>();
        }
        */

        return opt_type;
    }

    template
    <
        template<typename, typename, typename...> typename MethodTrait,

        typename T,

        typename ReturnType=T,
        typename OtherType=T,

        bool method_is_const=true,
        bool method_is_noexcept=false
    >
    constexpr bool has_meta_operator_method()
    {
        using Method = MethodTrait<T, ReturnType, const OtherType&>;

        if constexpr (Method::value)
        {
            constexpr auto ptr = Method::template ptr<method_is_const, method_is_noexcept>;

            if constexpr (std::is_same_v<std::nullptr_t, decltype(ptr)>)
            {
                return false;
            }
            else
            {
                return true;
            }
        }
        else
        {
            return false;
        }
    }

    template
    <
        template<typename, typename, typename...> typename FunctionTrait,

        typename T,

        typename ReturnType=T,
        typename OtherType=T
    >
    constexpr bool has_meta_operator_function()
    {
        using Function = FunctionTrait<T, ReturnType, const T&, const OtherType&>;

        if constexpr (Function::value)
        {
            constexpr auto ptr = Function::template ptr<>;

            if constexpr (std::is_same_v<std::nullptr_t, decltype(ptr)>)
            {
                return false;
            }
            else
            {
                return true;
            }
        }
        else
        {
            return false;
        }
    }

    template
    <
        template<typename, typename, typename...> typename FunctionTrait,
        template<typename, typename, typename...> typename MethodTrait,

        typename T,

        typename ReturnType=T,
        typename OtherType=T,

        bool method_is_const=true,
        bool method_is_noexcept=false
    >
    constexpr bool has_meta_operator()
    {
        if constexpr (has_meta_operator_method<MethodTrait, T, ReturnType, OtherType, method_is_const, method_is_noexcept>())
        {
            return true;
        }
        else if constexpr (has_meta_operator_function<FunctionTrait, T, ReturnType, OtherType>())
        {
            return true;
        }
        else
        {
            return false;
        }
    }

    template
    <
        template<typename, typename, typename...> typename MethodTrait,

        typename T,

        typename ReturnType=T,
        typename OtherType=const T&,

        bool method_is_const=true,
        bool method_is_noexcept=false
    >
    auto define_meta_operator_method(auto type, auto&& operator_id, bool* opt_success_flag=nullptr)
    {
        using Method = MethodTrait<T, ReturnType, OtherType>;

        if constexpr (Method::value)
        {
            constexpr auto ptr = Method::template ptr<method_is_const, method_is_noexcept>;
            
            static_assert(!std::is_same_v<std::nullptr_t, decltype(ptr)>, "Unable to resolve pointer to operator method.");

            if (opt_success_flag)
            {
                *opt_success_flag = true;
            }

            if constexpr (std::is_reference_v<ReturnType>)
            {
                if constexpr (std::is_const_v<std::remove_reference_t<ReturnType>>)
                {
                    return type.template func<ptr, entt::as_cref_t>(operator_id);
                }
                else
                {
                    return type.template func<ptr, entt::as_ref_t>(operator_id);
                }
            }
            else
            {
                return type.template func<ptr>(operator_id);
            }
        }
        else
        {
            if (opt_success_flag)
            {
                *opt_success_flag = false;
            }

            return type;
        }
    }

    template
    <
        template<typename, typename, typename...> typename FunctionTrait,

        typename T,

        typename ReturnType=T,
        typename OtherType=const T&
    >
    auto define_meta_operator_function(auto type, auto&& operator_id, bool* opt_success_flag=nullptr)
    {
        using Function = FunctionTrait<T, ReturnType, const T&, OtherType>;

        if constexpr (Function::value)
        {
            constexpr auto ptr = Function::template ptr<>;

            static_assert(!std::is_same_v<std::nullptr_t, decltype(ptr)>, "Unable to resolve pointer to operator function.");

            if (opt_success_flag)
            {
                *opt_success_flag = true;
            }

            if constexpr (std::is_reference_v<ReturnType>)
            {
                if constexpr (std::is_const_v<std::remove_reference_t<ReturnType>>)
                {
                    return type.template func<ptr, entt::as_cref_t>(operator_id);
                }
                else
                {
                    return type.template func<ptr, entt::as_ref_t>(operator_id);
                }
            }
            else
            {
                return type.template func<ptr>(operator_id);
            }
        }
        else
        {
            if (opt_success_flag)
            {
                *opt_success_flag = false;
            }

            return type;
        }
    }

    template
    <
        template<typename, typename, typename...> typename FunctionTrait,
        template<typename, typename, typename...> typename MethodTrait,

        typename T,

        typename ReturnType=T,
        typename OtherType=const T&,

        bool method_is_const=true,
        bool method_is_noexcept=false
    >
    auto define_meta_operator(auto type, auto&& operator_id, bool* opt_success_flag=nullptr)
    {
        if constexpr (has_meta_operator_method<MethodTrait, T, ReturnType, OtherType, method_is_const, method_is_noexcept>())
        {
            return define_meta_operator_method<MethodTrait, T, ReturnType, OtherType, method_is_const, method_is_noexcept>(type, operator_id, opt_success_flag);
        }
        else if constexpr (has_meta_operator_function<FunctionTrait, T, ReturnType, OtherType>())
        {
            return define_meta_operator_function<FunctionTrait, T, ReturnType, OtherType>(type, operator_id, opt_success_flag);
        }
        else
        {
            if (opt_success_flag)
            {
                *opt_success_flag = false;
            }

            return type;
        }
    }

    template
    <
        typename T,
        typename OtherType=const T&,

        bool method_is_const=true,
        bool method_is_noexcept=true
    >
    auto define_equality_operators(auto type)
    {
        constexpr bool has_equality_operator = has_meta_operator_method<has_method_operator_equal, T, bool, OtherType, method_is_const, method_is_noexcept>();
        constexpr bool has_inequality_operator = has_meta_operator_method<has_method_operator_not_equal, T, bool, OtherType, method_is_const, method_is_noexcept>();

        if constexpr (has_equality_operator)
        {
            type = define_meta_operator_method<has_method_operator_equal, T, bool, OtherType>(type, "operator=="_hs);

            if constexpr (!has_inequality_operator)
            {
                type = type.template func<&inequality_operator_fallback_impl<T, OtherType>>("operator!="_hs);
            }
        }
        
        if constexpr (has_inequality_operator)
        {
            type = define_meta_operator_method<has_method_operator_not_equal, T, bool, OtherType>(type, "operator!="_hs);

            if constexpr (!has_equality_operator)
            {
                type = type.template func<&equality_operator_fallback_impl<T, OtherType>>("operator=="_hs);
            }
        }

        return type;
    }

    template <typename T>
    auto define_boolean_operators(auto type)
    {
        // Check for contextual conversion to boolean.
        if constexpr (util::is_convertible_to_bool<T>)
        {
            return type
                .template func<&operator_bool_impl<T>>("operator bool"_hs)
                .template func<&operator_logical_not_impl<T>>("!operator"_hs)
            ;
        }
        else
        {
            return type;
        }
    }

    template
    <
        template<typename, typename, typename...> typename FunctionTrait,

        typename T,

        typename ReturnType=T,
        typename OtherType=T
    >
    constexpr bool has_unary_meta_operator(auto type, auto&& operator_id)
    {
        using Function = FunctionTrait<T, ReturnType, const T&>;

        if constexpr (Function::value)
        {
            constexpr auto ptr = Function::template ptr<>;

            if constexpr (std::is_same_v<std::nullptr_t, decltype(ptr)>)
            {
                return false;
            }
            else
            {
                return true;
            }
        }
        else
        {
            return true;
        }
    }

    template
    <
        template<typename, typename, typename...> typename FunctionTrait,

        typename T,

        typename ReturnType=T,
        typename OtherType=T
    >
    auto define_unary_meta_operator(auto type, auto&& operator_id)
    {
        using Function = FunctionTrait<T, ReturnType, const T&>;

        if constexpr (Function::value)
        {
            constexpr auto ptr = Function::template ptr<>;

            static_assert(!std::is_same_v<std::nullptr_t, decltype(ptr)>, "Unable to resolve pointer to unary operator function.");

            return type.template func<ptr>(operator_id);
        }
        else
        {
            return type;
        }
    }

    // NOTE: lvalue references are used in place of values or rvalue
    // references due to a limitation in EnTT's invocation code.
    // 
    // TODO: Look into changing last `MetaAny&` argument to `MetaAny&&`.
    template <typename T, typename ReturnType=T&> // MetaAny
    std::tuple<bool, bool, bool, bool, bool, bool> define_indirect_setters
    (
        auto& type,
        std::tuple<bool, bool, bool, bool, bool, bool> setters_found = { false, false, false, false, false, false }
    )
    {
        auto& basic_setter_found         = std::get<0>(setters_found);
        auto& basic_context_setter_found = std::get<1>(setters_found);
        auto& exact_setter_found         = std::get<2>(setters_found);
        auto& basic_chain_setter_found   = std::get<3>(setters_found);
        auto& context_chain_setter_found = std::get<4>(setters_found);
        auto& exact_chain_setter_found   = std::get<5>(setters_found);

        if constexpr (std::is_same_v<ReturnType, T&> || std::is_same_v<ReturnType, MetaAny>)
        {
            if (!basic_setter_found)
            {
                reflect_any_function
                <
                    T, false, false,
                    std::conditional_t<std::is_lvalue_reference_v<ReturnType>, entt::as_ref_t, entt::as_is_t>,
                    
                    has_method_set_indirect_value<T, ReturnType, MetaAny&>,
                    has_method_set<T, ReturnType, MetaAny&>
                >(type, "operator="_hs, basic_setter_found);
            }

            if (!basic_context_setter_found)
            {
                reflect_any_function
                <
                    T, false, false,
                    std::conditional_t<std::is_lvalue_reference_v<ReturnType>, entt::as_ref_t, entt::as_is_t>,
                    
                    has_method_set_indirect_value<T, ReturnType, MetaAny&, const MetaEvaluationContext&>,
                    has_method_set<T, ReturnType, MetaAny&, const MetaEvaluationContext&>
                >(type, "operator="_hs, basic_context_setter_found);
            }

            if (!exact_setter_found)
            {
                // `MetaEvaluationContext` overloads:
                reflect_any_function
                <
                    T, false, false,
                    std::conditional_t<std::is_lvalue_reference_v<ReturnType>, entt::as_ref_t, entt::as_is_t>,
                    
                    has_method_set_indirect_value<T, ReturnType, MetaAny&, Registry&, Entity, const MetaEvaluationContext&>,
                    has_method_set<T, ReturnType, MetaAny&, Registry&, Entity, const MetaEvaluationContext&>
                >(type, "operator="_hs, exact_setter_found);

                // Standard overloads (no `MetaEvaluationContext`):
                reflect_any_function
                <
                    T, false, false,
                    std::conditional_t<std::is_lvalue_reference_v<ReturnType>, entt::as_ref_t, entt::as_is_t>,
                    
                    has_method_set_indirect_value<T, ReturnType, MetaAny&, Registry&, Entity>,
                    has_method_set<T, ReturnType, MetaAny&, Registry&, Entity>
                >(type, "operator="_hs, exact_setter_found);
            }
        }

        if constexpr (std::is_same_v<ReturnType, void> || std::is_same_v<ReturnType, MetaAny> || std::is_same_v<ReturnType, Entity>)
        {
            if (!basic_chain_setter_found)
            {
                reflect_any_function
                <
                    T, false, false, entt::as_is_t,
                    
                    has_method_set_indirect_value<T, ReturnType, MetaAny&, MetaAny&>,
                    has_method_set<T, ReturnType, MetaAny&, MetaAny&>
                >(type, "operator="_hs, basic_chain_setter_found);
            }

            if (!context_chain_setter_found)
            {
                reflect_any_function
                <
                    T, false, false, entt::as_is_t,
                    
                    has_method_set_indirect_value<T, ReturnType, MetaAny&, MetaAny&, const MetaEvaluationContext&>,
                    has_method_set<T, ReturnType, MetaAny&, MetaAny&, const MetaEvaluationContext&>
                >(type, "operator="_hs, context_chain_setter_found);
            }

            if (!exact_chain_setter_found)
            {
                // `MetaEvaluationContext` overloads:
                reflect_any_function
                <
                    T, false, false, entt::as_is_t,
                    
                    has_method_set_indirect_value<T, ReturnType, MetaAny&, MetaAny&, Registry&, Entity, const MetaEvaluationContext&>,
                    has_method_set<T, ReturnType, MetaAny&, MetaAny&, Registry&, Entity, const MetaEvaluationContext&>
                >(type, "operator="_hs, exact_chain_setter_found);

                // Standard overloads (no `MetaEvaluationContext`):
                reflect_any_function
                <
                    T, false, false, entt::as_is_t,
                    
                    has_method_set_indirect_value<T, ReturnType, MetaAny&, MetaAny&, Registry&, Entity>,
                    has_method_set<T, ReturnType, MetaAny&, MetaAny&, Registry&, Entity>
                >(type, "operator="_hs, exact_chain_setter_found);
            }
        }

        return setters_found;
    }

    template <typename T, typename ReturnType>
    std::tuple<bool, bool, bool, bool, bool, bool> define_indirect_getters
    (
        auto& type,

        std::tuple<bool, bool, bool, bool, bool, bool> getters_found = { false, false, false, false, false, false }
    )
    {
        auto& basic_getter_found         = std::get<0>(getters_found);
        auto& basic_context_getter_found = std::get<1>(getters_found);
        auto& exact_getter_found         = std::get<2>(getters_found);
        auto& chain_getter_found         = std::get<3>(getters_found);
        auto& chain_context_getter_found = std::get<4>(getters_found);
        auto& exact_chain_getter_found   = std::get<5>(getters_found);

        if (!basic_getter_found)
        {
            reflect_any_function
            <
                T, true, false, entt::as_is_t,
                has_method_get_indirect_value<T, ReturnType>,
                has_method_get<T, ReturnType>
            >(type, "operator()"_hs, basic_getter_found);
        }

        if (!basic_context_getter_found)
        {
            reflect_any_function
            <
                T, true, false, entt::as_is_t,
                has_method_get_indirect_value<T, ReturnType, const MetaEvaluationContext&>,
                has_method_get<T, ReturnType, const MetaEvaluationContext&>
            >(type, "operator()"_hs, basic_context_getter_found);
        }

        if (!exact_getter_found)
        {
            // `MetaEvaluationContext` overloads:
            reflect_any_function
            <
                T, true, false, entt::as_is_t,
                
                has_method_get_indirect_value<T, ReturnType, Registry&, Entity, const MetaEvaluationContext&>,
                has_method_get<T, ReturnType, Registry&, Entity, const MetaEvaluationContext&>
            >(type, "operator()"_hs, exact_getter_found);

            // Standard overloads (no `MetaEvaluationContext`):
            reflect_any_function
            <
                T, true, false, entt::as_is_t,
                
                has_method_get_indirect_value<T, ReturnType, Registry&, Entity>,
                has_method_get<T, ReturnType, Registry&, Entity>
            >(type, "operator()"_hs, exact_getter_found);
        }

        if (!chain_getter_found)
        {
            reflect_any_function
            <
                T, true, false, entt::as_is_t,
                
                has_method_get_indirect_value<T, ReturnType, const MetaAny&>,
                has_method_get<T, ReturnType, const MetaAny&>
            >(type, "operator()"_hs, chain_getter_found);
        }

        if (!chain_context_getter_found)
        {
            reflect_any_function
            <
                T, true, false, entt::as_is_t,
                
                has_method_get_indirect_value<T, ReturnType, const MetaAny&, const MetaEvaluationContext&>,
                has_method_get<T, ReturnType, const MetaAny&, const MetaEvaluationContext&>
            >(type, "operator()"_hs, chain_context_getter_found);
        }

        if (!exact_chain_getter_found)
        {
            // `MetaEvaluationContext` overloads:
            reflect_any_function
            <
                T, true, false, entt::as_is_t,
                
                // From `MetaAny`:
                has_method_get_indirect_value<T, ReturnType, const MetaAny&, Registry&, Entity, const MetaEvaluationContext&>,
                has_method_get<T, ReturnType, const MetaAny&, Registry&, Entity, const MetaEvaluationContext&>,

                // From `Entity`:
                has_method_get_indirect_value<T, ReturnType, Entity, Registry&, Entity, const MetaEvaluationContext&>,
                has_method_get<T, ReturnType, Entity, Registry&, Entity, const MetaEvaluationContext&>
            >(type, "operator()"_hs, exact_chain_getter_found);

            // Standard overloads (no `MetaEvaluationContext`):

            reflect_any_function
            <
                T, true, false, entt::as_is_t,
                
                // From `MetaAny`:
                has_method_get_indirect_value<T, ReturnType, const MetaAny&, Registry&, Entity>,
                has_method_get<T, ReturnType, const MetaAny&, Registry&, Entity>,

                // From `Entity`:
                has_method_get_indirect_value<T, ReturnType, Entity, Registry&, Entity>,
                has_method_get<T, ReturnType, Entity, Registry&, Entity>
            >(type, "operator()"_hs, exact_chain_getter_found);
        }

        return getters_found;
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
            .template func<&from_meta<T>>("from_meta"_hs)
            .template func<&short_name<T>>("get_type_name"_hs)
            .template func<&trigger_event_from_meta_any<T>>("trigger_event_from_meta_any"_hs)
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

        if constexpr (!std::is_empty_v<T>)
        {
            type = type
                .template func<&has_component<T>>("has_component"_hs)
                .template func<&get_component<T>>("get_component"_hs)
                .template func<&emplace_meta_component<T>, entt::as_ref_t>("emplace_meta_component"_hs)
                .template func<&store_meta_component<T>>("store_meta_component"_hs)
                .template func<&copy_meta_component<T>>("copy_meta_component"_hs)
                .template func<&remove_component<T>>("remove_component"_hs)
                .template func<&get_or_emplace_component<T>, entt::as_ref_t>("get_or_emplace_component"_hs)
                .template func<&direct_patch_meta_component<T>, entt::as_ref_t>("direct_patch_meta_component"_hs)
                .template func<&indirect_patch_meta_component<T>>("indirect_patch_meta_component"_hs)
                .template func<&mark_component_as_patched<T>>("mark_component_as_patched"_hs)
			    .template func<&MetaEventListener::connect_component_listeners<T>>("connect_component_meta_events"_hs)
                .template func<&MetaEventListener::disconnect_component_listeners<T>>("disconnect_component_meta_events"_hs)
                .template func<&MetaEventListener::connect<T>>("connect_meta_event"_hs)
                .template func<&MetaEventListener::disconnect<T>>("disconnect_meta_event"_hs)
            ;
        }

        if constexpr ((config.generate_json_constructor) && (std::is_default_constructible_v<T>))
        {
            type = type
                .ctor
                <
                    static_cast<T (*)(const util::json&)>
                    (&from_json<T>)
                >()
                    
                .ctor
                <
                    static_cast<T (*)(const util::json&, const MetaParsingInstructions&)>
                    (&from_json_with_instructions<T>) // from_json<T>
                >()

                .ctor
                <
                    static_cast<T (*)(const util::json&, const MetaParsingInstructions&, const MetaTypeDescriptorFlags&)>
                    (&from_json_with_instructions_and_descriptor_flags<T>) // from_json<T>
                >()
            ;
        }

        if constexpr (config.generate_optional_reflection)
        {
            optional_engine_meta_type<T>();
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
    auto custom_meta_type(MetaTypeID type_id)
    {
        // Ensure that we're using the correct context.
        sync_reflection_context();

        auto type = entt::meta<T>().type(type_id);

        return define_standard_meta_type<T, config>(type, false);
    }

    // Associates a stripped version of the type's name to its reflection metadata.
    // Allows use of `T` without specifying `engine::T` in contexts where `T` is named dynamically. (e.g. JSON I/O)
    // 
    // By default, `entt` uses a fully qualified name, along with a "struct" or "class" prefix, etc.
    // This allows you to simply refer to the type by its namespace-local name.
    template <typename T, MetaTypeReflectionConfig config={}>
    auto engine_meta_type()
    {
        return custom_meta_type<T, config>(short_name_hash<T>());
    }

    template <typename T> // MetaTypeReflectionConfig config={}
    auto static_engine_meta_type()
    {
        // Ensure that we're using the correct context.
        sync_reflection_context();

        auto type = entt::meta<T>().type(short_name_hash<T>());

        return type;
    }

    template <typename T>
    auto engine_command_type()
    {
        return engine_meta_type<T, MetaTypeReflectionConfig { .capture_standard_data_members = false }>()
            .base<Command>()
        ;
    }

    // TODO: Look into this implementation again.
    // (Probably not very efficient to use properties for this)
    template <typename EnumType>
    void reflect_enum(MetaTypeID type_id, bool values_as_properties=false)
    {
        auto meta_obj = custom_meta_type<EnumType>(type_id)
            .template func<&string_to_enum_value<EnumType>>("string_to_value"_hs)
            .ctor<&string_to_enum_value<EnumType>>()
            .conv<&enum_value_to_string<EnumType>>()
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

    // NOTE: This is called automatically via `reflect` when `T` is an enumeration type.
    template <typename EnumType>
    void reflect_enum(bool values_as_properties=false)
    {
        reflect_enum<EnumType>(short_name_hash<EnumType>(), values_as_properties);
    }
}