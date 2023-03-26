#pragma once

#include <entt/meta/meta.hpp>
//#include <entt/entt.hpp>

#include "types.hpp"
#include "traits.hpp"
#include "enum.hpp"
#include "hash.hpp"
#include "indirection.hpp"
#include "container.hpp"
#include "meta_any_wrapper.hpp"

//#include "meta_function_call.hpp"

#include <engine/types.hpp>

#include <util/reflection.hpp>
#include <util/magic_enum.hpp>
#include <util/string.hpp>
#include <util/format.hpp>
#include <util/algorithm.hpp>
#include <util/small_vector.hpp>

#include <string>
#include <string_view>
#include <type_traits>
#include <optional>
#include <utility>
#include <array>
#include <tuple>

namespace engine
{
    constexpr std::array short_name_prefixes =
    {
        "struct engine::",
         "class engine::",
          "enum engine::",
         "union engine::"
    };

    // Resolves the meta-type for `type_name` as if it were part of the `engine` namespace.
    // (i.e. `type_name` should be a namespace-local identifier)
    entt::meta_type meta_type_from_name(std::string_view type_name);

    std::optional<std::pair<entt::id_type, entt::meta_data>>
    get_local_data_member_by_index(const entt::meta_type& type, std::size_t variable_index);

    std::optional<std::pair<entt::id_type, entt::meta_data>>
    get_data_member_by_index(const entt::meta_type& type, std::size_t variable_index, bool recursive=true);

    // Attempts to retrieve a `PlayerIndex` value from `instance`, if applicable.
    std::optional<PlayerIndex> resolve_player_index(const MetaAny& instance);

    bool meta_any_is_string(const MetaAny& value);
    bool meta_type_is_string(const MetaType& type);

	// Attempts to compute a string hash for `value`.
	std::optional<StringHash> meta_any_to_string_hash(const MetaAny& value);

	// Compares `left` and `right` to see if they both have the same string hash.
	bool meta_any_string_compare(const MetaAny& left, const MetaAny& right);

    // Retrieves a reference to an instance of `component_type` attached to `entity`.
    // If an instance could not be resolved, an empty `MetaAny` will be returned.
    MetaAny get_component_ref(Registry& registry, Entity entity, const MetaType& component_type);

    // Retrieves a reference to an instance of the type identified by `component_type_id` attached to `entity`.
    // If an instance could not be resolved, or if a type could not be resolve from `component_type_id`, an empty `MetaAny` will be returned.
    MetaAny get_component_ref(Registry& registry, Entity entity, MetaTypeID component_type_id);

    // Attempts to convert `current_value` to `cast_type` in-place.
    // If the type-cast was successful, this will return true.
    // If `current_value` is an indirect value, this will fail (return false).
    bool try_direct_cast(MetaAny& current_value, const MetaType& cast_type, bool ignore_same_type=true);

    bool function_overload_has_meta_any_argument(const MetaFunction& function);
	bool function_has_meta_any_argument(const MetaFunction& function, bool check_overloads=true);

    // Attempts to invoke `function` with the `args` specified.
    // 
    // If invocation fails, the next overload of `function` will be attempted.
    // If all overloads fail, this function will return an empty `MetaAny` object.
    // 
    // NOTE: This function encapsulates `args` via `entt::forward_as_meta` automatically.
	template <typename InstanceType, typename ...Args>
	MetaAny invoke_any_overload(MetaFunction function, InstanceType&& instance, Args&&... args)
	{
		static_assert(std::is_same_v<std::decay_t<InstanceType>, MetaAny>, "The `instance` parameter must be a `MetaAny` object, or a reference to one.");

		while (function)
		{
			// NOTE: Function `arity` check added to avoid unnecessary object creation overhead.
			if (function.arity() == sizeof...(args)) // >=
			{
				if (auto result = function.invoke(instance, entt::forward_as_meta(args)...))
				{
					return result;
				}
			}

			function = function.next();
		}

		return {};
	}

	template <typename InstanceType> // typename ...Args
	MetaAny invoke_any_overload(MetaFunction function, InstanceType&& instance, MetaAny* const args, std::size_t arg_count)
	{
		static_assert(std::is_same_v<std::decay_t<InstanceType>, MetaAny>, "The `instance` parameter must be a `MetaAny` object, or a reference to one.");

		while (function)
		{
			// NOTE: We don't bother with the function `arity` check here,
			// since EnTT already handles that. (+ Copying a pointer is essentially free)
            //if (arg_count == function.arity()) // >=
            {
			    if (auto result = function.invoke(instance, args, arg_count))
			    {
				    return result;
			    }
            }

			function = function.next();
		}

		return {};
	}

    // Thia function attempts to invoke an overload of `function` that takes `arg_count` from `args`.
    // If this fails, a second invocation pass will be attempted where any overload's parameters
    // of type `MetaAny` is handled via an `entt::forward_as_meta` passthrough.
    template <typename InstanceType>
	static MetaAny invoke_any_overload_with_automatic_meta_forwarding(MetaFunction function, InstanceType&& instance, MetaAny* const args, std::size_t arg_count)
	{
		static_assert(std::is_same_v<std::decay_t<InstanceType>, MetaAny>, "The `instance` parameter must be a `MetaAny` object, or a reference to one.");

		if (auto result = invoke_any_overload(function, instance, args, arg_count))
		{
			return result;
		}

		using TemporaryArgumentStore = util::small_vector<MetaAny, 8>;

		TemporaryArgumentStore forwarding_store;

		while (function)
		{
			const auto intended_arg_count = function.arity();

			if ((arg_count == intended_arg_count) && function_overload_has_meta_any_argument(function)) // >=
			{
				forwarding_store.reserve(intended_arg_count); // arg_count

				for (std::size_t i = 0; i < intended_arg_count; i++)
				{
					auto arg_type = function.arg(i);

					if (arg_type.id() == entt::type_hash<MetaAny>::value()) // resolve<MetaAny>().id()
					{
						forwarding_store.emplace_back(entt::forward_as_meta(args[i].as_ref()));
					}
					else
					{
						forwarding_store.emplace_back(args[i].as_ref());
					}
				}

				// NOTE: Const-cast needed due to limitations of EnTT's API.
				if (auto result = function.invoke(instance, const_cast<MetaAny* const>(forwarding_store.data()), forwarding_store.size()))
				{
					return result;
				}

				forwarding_store.clear();
			}

			function = function.next();
		}

		return {};
	}

	// Shortens the name of `T` if `T` belongs to the `engine` namespace.
    // 
    // See also: `engine_meta_type`
	template <typename T>
    constexpr std::string_view short_name()
    {
        return std::apply
        (
            [](auto&&... prefixes)
            {
                return util::resolve_short_name<T>(std::forward<decltype(prefixes)>(prefixes)...);
            },

            short_name_prefixes
        );
    }

    constexpr std::string optional_name(const auto& type_name)
    {
        return util::format("std::optional<{}>", type_name);
    }

    template <typename T>
    constexpr std::string optional_short_name()
    {
        return optional_name(short_name<T>());
    }

    constexpr std::string_view as_short_name(std::string_view name_view)
    {
        return std::apply
        (
            [&name_view](auto&&... names)
            {
                return util::as_short_name(name_view, std::forward<decltype(names)>(names)...);
            },

            short_name_prefixes
        );
    }

    // Computes the hash associated with a `short_name`.
    // Useful for identifying a name local to the `engine` namespace by its opaque hashed value.
    // See also: `short_name`, `engine_meta_type`
    template <typename T>
    constexpr auto short_name_hash()
    {
        //return entt::type_hash<T>();

        return hash(short_name<T>());
    }

    template <typename T>
    constexpr auto optional_short_name_hash()
    {
        return hash(optional_short_name<T>());
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

	// Retrieves the runtime (property-based) value associated to `id` in `meta_type_inst`.
    // If no value is associated, this will fail via assertion.
    template <typename EnumType>
    EnumType get_reflected_enum(const entt::meta_type& meta_type_inst, entt::hashed_string id)
    {
        auto meta_property = meta_type_inst.prop(id);
        return meta_property.value().cast<EnumType>();
    }

    // Computes the hash of `enum_value_name`, then retrieves the runtime value associated by `meta_type_inst`.
    // If no value is associated, this will fail via assertion.
    template <typename EnumType>
    EnumType get_reflected_enum(const entt::meta_type& meta_type_inst, std::string_view enum_value_name)
    {
        return get_reflected_enum<EnumType>(meta_type_inst, entt::hashed_string(enum_value_name.data(), enum_value_name.size()));
    }

    // Retrieves a reflected enum value by its name or hash of its name.
    // If no value is associated (see `reflect_enum` and `reflect`), this will fail via assertion.
    template <typename EnumType, typename IdentifierType>
    EnumType get_reflected_enum(const IdentifierType& enum_value_identifier)
    {
        return get_reflected_enum<EnumType>(entt::resolve<EnumType>(), enum_value_identifier);
    }

    template <typename Callback>
    inline bool enumerate_data_members(const entt::meta_type& type, Callback&& callback, bool recursive=true, std::size_t* count_out=nullptr)
    {
        if (recursive)
        {
            for (const auto& base_type_entry : type.base())
            {
                const auto& base_type = std::get<1>(base_type_entry);

                if (!enumerate_data_members(base_type, callback, recursive, count_out))
                {
                    break;
                }
            }
        }

        std::size_t count = 0;
        bool result = true;

        for (auto&& entry : type.data())
        {
            //callback(std::forward<decltype(entry)>(entry));

            const auto& data_member_id = entry.first;
            const auto& data_member    = entry.second;

            result = callback(data_member_id, data_member);

            count++;

            if (!result)
            {
                break;
            }
        }

        if (count_out)
        {
            *count_out += count;
        }

        return result;
    }

    template <typename MemberID=MetaSymbolID>
    inline entt::meta_data resolve_data_member_by_id(const entt::meta_type& type, bool check_base_types, MemberID member_name_id)
    {
        auto data_member = type.data(member_name_id);

        if (!data_member)
        {
            if (check_base_types)
            {
                for (const auto& base_type : type.base())
                {
                    data_member = resolve_data_member_by_id(base_type.second, check_base_types, member_name_id); // true

                    if (data_member)
                    {
                        break;
                    }
                }
            }
        }

        return data_member;
    }

    template <typename MemberID, typename ...MemberIDs> // MetaSymbolID
    inline entt::meta_data resolve_data_member_by_id(const entt::meta_type& type, bool check_base_types, MemberID&& member_name_id, MemberIDs&&... member_name_ids)
    {
        if (auto data_member = resolve_data_member_by_id(type, check_base_types, member_name_id))
        {
            return data_member;
        }

        return resolve_data_member_by_id(type, check_base_types, std::forward<MemberIDs>(member_name_ids)...);
    }

    template <typename MemberName=std::string_view>
    inline std::tuple<MetaSymbolID, entt::meta_data> resolve_data_member(const entt::meta_type& type, bool check_base_types, MemberName&& member_name)
    {
        if (std::string_view(member_name).empty())
        {
            return {};
        }

        const auto member_name_id = hash(member_name);
        auto data_member = resolve_data_member_by_id(type, check_base_types, member_name_id);
        
        return { member_name_id, data_member };
    }

    template <typename MemberName, typename ...MemberNames> // std::string_view
    inline std::tuple<MetaSymbolID, entt::meta_data> resolve_data_member(const entt::meta_type& type, bool check_base_types, MemberName&& member_name, MemberNames&&... member_names)
    {
        if (auto data_member = resolve_data_member(type, check_base_types, member_name); std::get<1>(data_member))
        {
            return data_member;
        }

        return resolve_data_member(type, check_base_types, std::forward<MemberNames>(member_names)...);
    }

    template <typename ...MemberNames> // std::string_view
    inline entt::meta_data resolve_data_member(const entt::meta_type& type, std::string_view member_name, MemberNames&&... member_names)
    {
        auto result = resolve_data_member(type, true, member_name, std::forward<MemberNames>(member_names)...);

        return std::get<1>(result);
    }

    template <typename T>
    inline bool set_data_member(MetaAny& instance, std::string_view member_name, T&& member_value)
    {
        auto type = instance.type();

        if (!type)
        {
            return false;
        }

        if (auto member = resolve_data_member(type, member_name))
        {
            return member.set(instance, member_value);

            //return true;
        }

        return false;
    }

	// Attempts to retrieve an instance of `Type` from `value`, passing it to `callback` if successful.
	template <typename Type, typename Callback>
	inline bool try_value(const MetaAny& value, Callback&& callback)
	{
		if (!value)
		{
			return false;
		}

		auto type = value.type();

		if (!type)
		{
			return false;
		}

		const auto type_id = type.id();

		//if (type_id == entt::type_id<Type>().hash())
		if ((type_id == entt::type_id<Type>().hash()) || (type_id == resolve<Type>().id()))
		{
			auto* value_raw = value.try_cast<Type>(); // const

			//assert(value_raw);

			if (!value_raw)
			{
				return false;
			}

			callback(*value_raw);

			return true;
		}

		return false;
	}

	template <typename Callback>
	inline bool try_string_value(const MetaAny& value, Callback&& callback)
	{
		if (try_value<std::string>(value, callback))
		{
			return true;
		}

		if (try_value<std::string_view>(value, callback))
		{
			return true;
		}

		return false;
	}

	/*
		Generates reflection meta-data for the `engine` module.
		Reflected meta-data can be queried via `entt`'s meta-type system.
		
		NOTES:
			* Although this is declared here, the function definition can be found in the `reflection` submodule.
			(This avoids multiple template instantiations)

			* Calling the `engine::reflect` free-function with `T=void` or with no
			template-specification will also result in a call this function.

			* Calling this function multiple times on the same thread is safe.
			
			* Calling this function from more than one thread concurrently is considered unsafe.
			(TODO: Look into fixing this)

			* The `Game` type automatically calls this function during construction.
			
			* The implementation of this function dictates the order-of-operations for reflected types.
			It is recommended that you do not inspect reflected types whilst meta factories are still being constructed.

			* The `primitives` argument is used to control whether we reflect simple enumeration types, structs, etc.
			* The `dependencies` argument determines if other supporting modules (e.g. `math`) are reflected.
	*/
	void reflect_all(bool primitives=true, bool dependencies=true);
}