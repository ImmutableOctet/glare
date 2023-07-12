#pragma once

#include "common_extensions.hpp"

#include <engine/types.hpp>
#include <engine/registry.hpp>

#include <engine/meta/types.hpp>
#include <engine/meta/cast.hpp>
#include <engine/meta/short_name.hpp>
#include <engine/meta/meta_type_descriptor.hpp>

#include <utility>
#include <type_traits>

namespace engine::impl
{
	template <typename T>
    T& emplace_meta_component(Registry& registry, Entity entity, MetaAny& value) // MetaAny&&
    {
        auto raw_value = from_meta<T>(value);

        if (!raw_value)
        {
            if (auto type = resolve<T>())
            {
                if (try_direct_cast(value, type))
                {
                    raw_value = from_meta<T>(value);
                }
            }
        }

        if (raw_value)
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
    T* get_or_default_construct_component(Registry& registry, Entity entity)
    {
        return &(registry.get_or_emplace<T>(entity));
    }

    template <typename T>
    T* get_component(Registry& registry, Entity entity)
    {
        return registry.try_get<T>(entity);
    }

    template <typename T>
    T* emplace_default_component(Registry& registry, Entity entity)
    {
        // Alternative implementation (Doesn't fail on existing):
        //return &(registry.get_or_emplace<T>(entity));

        if (auto existing = get_component<T>(registry, entity))
        {
            return nullptr; // existing;
        }

        return &(registry.emplace<T>(entity));
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
    bool mark_component_as_patched(Registry& registry, Entity entity)
    {
        if (registry.try_get<T>(entity))
        {
            registry.patch<T>(entity);

            return true;
        }

        return false;
    }

    template <typename T>
    MetaTypeID get_component_type_id_impl()
    {
        return short_name_hash<T>();
    }

    template <typename T>
    MetaTypeID get_history_component_type_id_impl()
    {
        return history_component_short_name_hash<T>();
    }
}