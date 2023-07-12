#pragma once

#include "types.hpp"
#include "runtime_traits.hpp"
#include "indirection.hpp"
#include "hash.hpp"

#include <optional>
#include <utility>
#include <type_traits>

namespace engine
{
	// Returns true if `data_member` cannot be written to.
	bool data_member_is_read_only(const entt::meta_data& data_member);

	std::optional<std::pair<entt::id_type, entt::meta_data>>
    get_local_data_member_by_index(const MetaType& type, std::size_t variable_index);

    std::optional<std::pair<entt::id_type, entt::meta_data>>
    get_data_member_by_index(const MetaType& type, std::size_t variable_index, bool recursive=true);

    // Attempts to retrieve a `PlayerIndex` value from `instance`, if applicable.
    std::optional<PlayerIndex> resolve_player_index(const MetaAny& instance);

    // NOTE: This routine adds to `count_out`, rather than assigning it.
    // Please be sure to initialize the integer before passing it by pointer to this function.
    template <typename Callback>
    bool enumerate_data_members
	(
		const MetaType& type,
		
		Callback&& callback,

		bool include_base_types=true,
		std::size_t* count_out=nullptr,
		bool allow_short_circuit=true
	)
    {
        if (!type)
        {
            return false;
        }

        if (include_base_types)
        {
            for (const auto& base_type_entry : type.base())
            {
                const auto& base_type = std::get<1>(base_type_entry);

                if (!enumerate_data_members(base_type, callback, include_base_types, count_out))
                {
					if (allow_short_circuit)
					{
						break;
					}
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
				if (allow_short_circuit)
				{
					break;
				}
            }
        }

        if (count_out)
        {
            *count_out += count;
        }

        return result;
    }

	// Counts the number of members available in `type`.
	inline std::size_t count_data_members(const MetaType& type, bool include_base_types=true)
	{
		std::size_t member_count = 0;

		enumerate_data_members
		(
			type,
			[](auto&&...) { return true; },
			include_base_types,
			&member_count,
			false
		);

		return member_count;
	}

	inline bool meta_type_has_data_member(const MetaType& type, bool include_base_types=true)
	{
		bool has_member = false;

		enumerate_data_members
		(
			type,
			
			[&has_member](auto&&...)
			{
				has_member = true;

				return false;
			},

			include_base_types,
			nullptr,
			true
		);

		return has_member;
	}

    /*
        Enumerates primitive members of `type`, calling `primitive_callback` for each.

        The `primitive_callback` parameter may be callable using one of the following signatures:
            * [bool](const MetaType& type, MetaSymbolID field_id, const MetaType& field_type, const entt::meta_data& field)
            * [bool](const MetaType& type, MetaSymbolID field_id, const MetaType& field_type)
            * [bool](const MetaType& type, MetaSymbolID field_id, const entt::meta_data& field)
        
        The `strings_as_primitives` parameter controls whether strings and string views are included as primitive types.
        The `enumerate_submembers` parameter determines whether members-of-members will be enumerated recursively.
        The `include_base_types` parameter controls whether base-types are enumerated for possible member entries.
    */
    template <typename PrimitiveCallback>
	void enumerate_primitive_data_members
    (
        const MetaType& type,

		PrimitiveCallback&& primitive_callback,
        
		bool strings_as_primitives=true,
		bool enumerate_submembers=true,
		bool include_base_types=true,
		bool allow_short_circuit=true
    )
	{
		enumerate_data_members
		(
			type,
									
			[&](MetaSymbolID field_id, const entt::meta_data& field)
			{
				const auto field_type = field.type();

				if (type_is_primitive(field_type))
				{
					if constexpr (std::is_invocable_r_v<bool, PrimitiveCallback, decltype(type), decltype(field_id), decltype(field_type), decltype(field)>)
					{
						if (!primitive_callback(type, field_id, field_type, field))
						{
							return false;
						}
					}
					else if constexpr (std::is_invocable_v<PrimitiveCallback, decltype(type), decltype(field_id), decltype(field_type), decltype(field)>)
					{
						primitive_callback(type, field_id, field_type, field);
					}
					else if constexpr (std::is_invocable_r_v<bool, PrimitiveCallback, decltype(type), decltype(field_id), decltype(field_type)>)
					{
						if (!primitive_callback(type, field_id, field_type))
						{
							return false;
						}
					}
					else if constexpr (std::is_invocable_v<PrimitiveCallback, decltype(type), decltype(field_id), decltype(field_type)>)
					{
						primitive_callback(type, field_id, field_type);
					}
					else if constexpr (std::is_invocable_r_v<bool, PrimitiveCallback, decltype(type), decltype(field_id), decltype(field)>)
					{
						if (!primitive_callback(type, field_id, field))
						{
							return false;
						}
					}
					else if constexpr (std::is_invocable_v<PrimitiveCallback, decltype(type), decltype(field_id), decltype(field)>)
					{
						primitive_callback(type, field_id, field);
					}
				}
				else
				{
					if (enumerate_submembers)
					{
						// NOTE: Recursion.
                        enumerate_primitive_data_members
						(
							field_type, primitive_callback,
							strings_as_primitives,
							enumerate_submembers,
							include_base_types,
							allow_short_circuit
						);
					}
				}

				return true;
			},

			include_base_types,
			nullptr,
			allow_short_circuit
		);
	}

    /*
		Enumerates the members of `instance` as opaque values (`MetaAny` objects),
		calling `primitive_callback` for primitive values and `non_primitive_callback` for non-primitive values.

		The `primitive_callback` argument should be a callable object with one of the following signatures:
		[bool]([const] MetaAny& instance, MetaSymbolID field_id, const entt::meta_data& field, [const] MetaAny& member_value)
		[bool]([const] MetaAny& instance, MetaSymbolID field_id, const entt::meta_data& field, [const] MetaAny& member_value, bool& instance_modified_out)

		The `non_primitive_callback` argument should be a callable object with one of the following signatures:
		[bool]([const] MetaAny& instance, MetaSymbolID field_id, const entt::meta_data& field, [const] MetaAny& member_value)
		[bool]([const] MetaAny& instance, MetaSymbolID field_id, const entt::meta_data& field, [const] MetaAny& member_value, bool& instance_modified_out)

		The `strings_as_primitives` parameter controls whether strings and string views are handled as primitive values.
		The `include_base_types` parameter controls whether base-types' member entries are included in data-member resolution.
	*/
	template <typename InstanceType, typename PrimitiveMetaCallback, typename NonPrimitiveMetaCallback>
	bool enumerate_opaque_primitive_member_values_ex
	(
		InstanceType&& instance,
		
		PrimitiveMetaCallback&& primitive_callback,
		NonPrimitiveMetaCallback&& non_primitive_callback,

		bool strings_as_primitives=true,
		bool include_base_types=true,
		bool allow_short_circuit=true,
		bool* opt_member_modified_out=nullptr
	)
	{
		static_assert(std::is_same_v<std::decay_t<InstanceType>, MetaAny>);

		const auto type = instance.type();

		bool exited_early = false;

		// Primitive values:
		enumerate_data_members
		(
			type,
									
			[&](MetaSymbolID field_id, const entt::meta_data& field)
			{
				const auto field_type = field.type();

				if (type_is_primitive(field_type))
				{
					auto member_value = field.get(instance);

					bool member_is_modified = false;

					if constexpr (std::is_invocable_r_v<bool, PrimitiveMetaCallback, decltype(instance), decltype(field_id), decltype(field), decltype(member_value), bool&>)
					{
						if (!primitive_callback(instance, field_id, field, member_value, member_is_modified))
						{
							exited_early = true;

							return false;
						}
					}
					else if constexpr (std::is_invocable_v<PrimitiveMetaCallback, decltype(instance), decltype(field_id), decltype(field), decltype(member_value), bool&>)
					{
						primitive_callback(instance, field_id, field, member_value, member_is_modified);
					}
					else if constexpr (std::is_invocable_r_v<bool, PrimitiveMetaCallback, decltype(instance), decltype(field_id), decltype(field), decltype(member_value)>)
					{
						if (!primitive_callback(instance, field_id, field, member_value))
						{
							exited_early = true;

							return false;
						}
					}
					else
					{
						primitive_callback(instance, field_id, field, member_value);
					}

					if (member_is_modified)
					{
						if (field.set(instance, member_value))
						{
							if (opt_member_modified_out)
							{
								*opt_member_modified_out = true;
							}
						}
					}
				}

				return true;
			},

			include_base_types,
			nullptr,
			allow_short_circuit
		);

		if (!exited_early)
		{
			// Non-primitive values:
			enumerate_data_members
			(
				type,
									
				[&](MetaSymbolID field_id, const entt::meta_data& field)
				{
					const auto field_type = field.type();

					if (!type_is_primitive(field_type))
					{
						auto member_value = field.get(instance);

						bool member_is_modified = false;

						if constexpr (std::is_invocable_r_v<bool, NonPrimitiveMetaCallback, decltype(instance), decltype(field_id), decltype(field), decltype(member_value), bool&>)
						{
							if (!non_primitive_callback(instance, field_id, field, member_value, member_is_modified))
							{
								exited_early = true;

								return false;
							}
						}
						else if constexpr (std::is_invocable_v<NonPrimitiveMetaCallback, decltype(instance), decltype(field_id), decltype(field), decltype(member_value), bool&>)
						{
							non_primitive_callback(instance, field_id, field, member_value, member_is_modified);
						}
						else if constexpr (std::is_invocable_r_v<bool, NonPrimitiveMetaCallback, decltype(instance), decltype(field_id), decltype(field), decltype(member_value)>)
						{
							if (!non_primitive_callback(instance, field_id, field, member_value))
							{
								exited_early = true;

								return false;
							}
						}
						else
						{
							non_primitive_callback(instance, field_id, field, member_value);
						}

						if (member_is_modified)
						{
							if (field.set(instance, member_value))
							{
								if (opt_member_modified_out)
								{
									*opt_member_modified_out = true;
								}
							}
						}
					}

					return true;
				},

				include_base_types,
				nullptr,
				allow_short_circuit
			);
		}

		return (!exited_early);
	}

	/*
		Enumerates the members of `instance` as opaque values (`MetaAny` objects), calling `primitive_callback` for primitive values.

		The `primitive_callback` argument should be a callable object with one of the following signatures:
		[bool]([const] MetaAny& instance, MetaSymbolID field_id, const entt::meta_data& field, [const] MetaAny& member_value)
		[bool]([const] MetaAny& instance, MetaSymbolID field_id, const entt::meta_data& field, [const] MetaAny& member_value, bool& instance_modified_out)

		The `non_primitive_callback_begin` and `non_primitive_callback_end` arguments should be callable objects with one of the following signatures:
		[bool]([const] MetaAny& instance, MetaSymbolID field_id, const entt::meta_data& field, [const] MetaAny& member_value)
		[bool]([const] MetaAny& instance, MetaSymbolID field_id, const entt::meta_data& field, [const] MetaAny& member_value, bool& instance_modified_out)

		The `strings_as_primitives` parameter controls whether strings and string views are handled as primitive values.
		The `enumerate_submembers` parameter controls whether recursive enumeration is performed when a non-primitive member is found.
		The `include_base_types` parameter controls whether base-types' member entries are included in data-member resolution.
	*/
	template <typename InstanceType, typename PrimitiveMetaCallback, typename NonPrimitiveMetaCallbackBegin, typename NonPrimitiveMetaCallbackEnd>
	bool enumerate_opaque_primitive_member_values
	(
		InstanceType&& instance,
		
		PrimitiveMetaCallback&& primitive_callback,
		
		NonPrimitiveMetaCallbackBegin&& non_primitive_callback_begin,
		NonPrimitiveMetaCallbackEnd&& non_primitive_callback_end,

		bool strings_as_primitives=true,
		bool enumerate_submembers=false,
		bool include_base_types=true,
		bool allow_short_circuit=true,
		bool* opt_member_modified_out=nullptr
	)
	{
		return enumerate_opaque_primitive_member_values_ex
		(
			instance, // std::forward<InstanceType>(instance),

			primitive_callback,

			[&](auto&& non_primitive, MetaSymbolID non_primitive_field_id, const entt::meta_data& non_primitive_field, auto&& non_primitive_member_value, bool& non_primitive_member_modified) -> bool
			{
				if constexpr (std::is_invocable_r_v<bool, NonPrimitiveMetaCallbackBegin, decltype(non_primitive), decltype(non_primitive_field_id), decltype(non_primitive_field), decltype(non_primitive_member_value), bool&>)
				{
					if (!non_primitive_callback_begin(non_primitive, non_primitive_field_id, non_primitive_field, non_primitive_member_value, non_primitive_member_modified))
					{
						return false;
					}
				}
				else if constexpr (std::is_invocable_v<NonPrimitiveMetaCallbackBegin, decltype(non_primitive), decltype(non_primitive_field_id), decltype(non_primitive_field), decltype(non_primitive_member_value), bool&>)
				{
					non_primitive_callback_begin(non_primitive, non_primitive_field_id, non_primitive_field, non_primitive_member_value);
				}
				else if constexpr (std::is_invocable_r_v<bool, NonPrimitiveMetaCallbackBegin, decltype(non_primitive), decltype(non_primitive_field_id), decltype(non_primitive_field), decltype(non_primitive_member_value)>)
				{
					if (!non_primitive_callback_begin(non_primitive, non_primitive_field_id, non_primitive_field, non_primitive_member_value))
					{
						return false;
					}
				}
				else
				{
					non_primitive_callback_begin(non_primitive, non_primitive_field_id, non_primitive_field, non_primitive_member_value);
				}

				bool recursive_result = true;

				if (enumerate_submembers)
				{
					// NOTE: Recursion.
					recursive_result = enumerate_opaque_primitive_member_values
					(
						non_primitive_member_value,
						primitive_callback,
						strings_as_primitives, enumerate_submembers, include_base_types, allow_short_circuit,
						&non_primitive_member_modified
					);
				}

				if constexpr (std::is_invocable_r_v<bool, NonPrimitiveMetaCallbackEnd, decltype(non_primitive), decltype(non_primitive_field_id), decltype(non_primitive_field), decltype(non_primitive_member_value), bool&>)
				{
					if (!non_primitive_callback_end(non_primitive, non_primitive_field_id, non_primitive_field, non_primitive_member_value, non_primitive_member_modified))
					{
						return false;
					}
				}
				else if constexpr (std::is_invocable_v<NonPrimitiveMetaCallbackEnd, decltype(non_primitive), decltype(non_primitive_field_id), decltype(non_primitive_field), decltype(non_primitive_member_value), bool&>)
				{
					non_primitive_callback_end(non_primitive, non_primitive_field_id, non_primitive_field, non_primitive_member_value, non_primitive_member_modified);
				}
				else if constexpr (std::is_invocable_r_v<bool, NonPrimitiveMetaCallbackEnd, decltype(non_primitive), decltype(non_primitive_field_id), decltype(non_primitive_field), decltype(non_primitive_member_value)>)
				{
					if (!non_primitive_callback_end(non_primitive, non_primitive_field_id, non_primitive_field, non_primitive_member_value))
					{
						return false;
					}
				}
				else
				{
					non_primitive_callback_end(non_primitive, non_primitive_field_id, non_primitive_field, non_primitive_member_value);
				}

				return recursive_result;
			},

			strings_as_primitives,
			include_base_types,
			allow_short_circuit,
			opt_member_modified_out
		);
	}

	// See primary overload for details.
	template <typename InstanceType, typename PrimitiveMetaCallback>
	bool enumerate_opaque_primitive_member_values
	(
		InstanceType&& instance,

		PrimitiveMetaCallback&& primitive_callback,
		
		bool strings_as_primitives=true,
		bool enumerate_submembers=false,
		bool include_base_types=true,
		bool allow_short_circuit=true,

		bool* opt_member_modified_out=nullptr
	)
	{
		return enumerate_opaque_primitive_member_values
		(
			instance,
			std::forward<PrimitiveMetaCallback>(primitive_callback),
			[](auto&&...) { /* Empty implementation. */ },
			[](auto&&...) { /* Empty implementation. */ },
			strings_as_primitives, enumerate_submembers, include_base_types, allow_short_circuit,
			opt_member_modified_out
		);
	}

	/*
		Enumerates the primitive members of `instance` as non-opaque values (i.e. the underlying types), calling `primitive_callback` for each.

		The `primitive_callback` argument should be a callable object with one of the following signatures:
		[bool]([const] MetaAny& instance, MetaSymbolID field_id, const entt::meta_data& field, auto&& member_value)
		[bool]([const] MetaAny& instance, MetaSymbolID field_id, const entt::meta_data& field, auto&& member_value, bool& instance_modified_out)

		When a non-primitive value is encountered, `non_primitive_callback_begin` will be executed,
		followed optionally by a call to `non_primitive_callback_end`.

		The `non_primitive_callback_begin` and `non_primitive_callback_end` arguments should be callable objects with one of the following signatures:
		[bool]([const] MetaAny& instance, MetaSymbolID field_id, const entt::meta_data& field, [const] MetaAny& member_value)
		[bool]([const] MetaAny& instance, MetaSymbolID field_id, const entt::meta_data& field, [const] MetaAny& member_value, bool& instance_modified_out)

		The `strings_as_primitives` parameter controls whether strings and string views are handled as primitive values.
		The `enumerate_submembers` parameter controls whether recursive enumeration is performed when a non-primitive member is found.
		The `include_base_types` parameter controls whether base-types' member entries are included in data-member resolution.
	*/
	template <typename InstanceType, typename PrimitiveCallback, typename NonPrimitiveMetaCallbackBegin, typename NonPrimitiveMetaCallbackEnd>
	bool enumerate_primitive_member_values
	(
		InstanceType&& instance,
		
		PrimitiveCallback&& primitive_callback,
		
		NonPrimitiveMetaCallbackBegin&& non_primitive_callback_begin,
		NonPrimitiveMetaCallbackEnd&& non_primitive_callback_end,
		
		bool strings_as_primitives=true,
		bool enumerate_submembers=false,
		bool include_base_types=true,
		bool allow_short_circuit=true,

		bool* opt_member_modified_out=nullptr
	)
	{
		using namespace engine::literals;

		return enumerate_opaque_primitive_member_values
		(
			instance, // std::forward<InstanceType>(instance),
			
			[&primitive_callback](auto&& instance, MetaSymbolID field_id, const entt::meta_data& field, auto&& member_value, auto&&... remaining_args) -> bool
			{
				bool result = true;

				const auto is_primitive = try_get_primitive_value
				(
					member_value, // std::forward<decltype(member_value)>(member_value),

					[&](auto&& underlying_value)
					{
						if constexpr (std::is_invocable_r_v<bool, PrimitiveCallback, decltype(instance), decltype(field_id), decltype(field), decltype(underlying_value), decltype(remaining_args)...>)
						{
							result = primitive_callback(instance, field_id, field, std::forward<decltype(underlying_value)>(underlying_value), std::forward<decltype(remaining_args)>(remaining_args)...);
						}
						else if constexpr (std::is_invocable_v<PrimitiveCallback, decltype(instance), decltype(field_id), decltype(field), decltype(underlying_value), decltype(remaining_args)...>)
						{
							primitive_callback(instance, field_id, field, std::forward<decltype(underlying_value)>(underlying_value), std::forward<decltype(remaining_args)>(remaining_args)...);
						}
						else
						{
							primitive_callback(instance, field_id, field, std::forward<decltype(underlying_value)>(underlying_value));
						}
					}
				);

				assert(is_primitive);

				return result;
			},

			std::forward<NonPrimitiveMetaCallbackBegin>(non_primitive_callback_begin),
			std::forward<NonPrimitiveMetaCallbackEnd>(non_primitive_callback_end),

			strings_as_primitives, enumerate_submembers, include_base_types, allow_short_circuit,

			opt_member_modified_out
		);
	}

	// See primary overload for details.
	template <typename InstanceType, typename PrimitiveCallback>
	bool enumerate_primitive_member_values
	(
		InstanceType&& instance,
		
		PrimitiveCallback&& primitive_callback,
		
		bool strings_as_primitives=true,
		bool enumerate_submembers=false,
		bool include_base_types=true,
		bool allow_short_circuit=true,

		bool* opt_member_modified_out=nullptr
	)
	{
		return enumerate_primitive_member_values
		(
			std::forward<InstanceType>(instance),
			std::forward<PrimitiveCallback>(primitive_callback),
			[](auto&&...) { /* Empty implementation. */ },
			[](auto&&...) { /* Empty implementation. */ },
			strings_as_primitives, enumerate_submembers, include_base_types, allow_short_circuit,
			opt_member_modified_out
		);
	}

    template <typename MemberID=MetaSymbolID>
    inline entt::meta_data resolve_data_member_by_id(const MetaType& type, bool check_base_types, MemberID member_name_id)
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
    inline entt::meta_data resolve_data_member_by_id(const MetaType& type, bool check_base_types, MemberID&& member_name_id, MemberIDs&&... member_name_ids)
    {
        if (auto data_member = resolve_data_member_by_id(type, check_base_types, member_name_id))
        {
            return data_member;
        }

        return resolve_data_member_by_id(type, check_base_types, std::forward<MemberIDs>(member_name_ids)...);
    }

    template <typename MemberName=std::string_view>
    inline std::tuple<MetaSymbolID, entt::meta_data> resolve_data_member(const MetaType& type, bool check_base_types, MemberName&& member_name)
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
    inline std::tuple<MetaSymbolID, entt::meta_data> resolve_data_member(const MetaType& type, bool check_base_types, MemberName&& member_name, MemberNames&&... member_names)
    {
        if (auto data_member = resolve_data_member(type, check_base_types, member_name); std::get<1>(data_member))
        {
            return data_member;
        }

        return resolve_data_member(type, check_base_types, std::forward<MemberNames>(member_names)...);
    }

    template <typename ...MemberNames> // std::string_view
    inline entt::meta_data resolve_data_member(const MetaType& type, std::string_view member_name, MemberNames&&... member_names)
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
}