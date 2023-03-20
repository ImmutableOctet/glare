#pragma once

#include <entt/meta/meta.hpp>
//#include <entt/entt.hpp>

#include "traits.hpp"
#include "enum.hpp"
#include "types.hpp"
#include "hash.hpp"
#include "meta_any_wrapper.hpp"

//#include "meta_function_call.hpp"

#include <engine/types.hpp>

#include <util/reflection.hpp>
#include <util/magic_enum.hpp>
#include <util/string.hpp>
#include <util/format.hpp>
#include <util/algorithm.hpp>

#include <string>
#include <string_view>
#include <type_traits>
#include <optional>
#include <utility>
#include <array>
#include <tuple>

namespace engine
{
    struct MetaEvaluationContext;

    struct MetaFunctionCall;
    struct MetaTypeReference;
    struct MetaDataMember;
    struct IndirectMetaDataMember;

    using entt::resolve;

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

    bool meta_any_is_string(const entt::meta_any& value);

	// Attempts to compute a string hash for `value`.
	std::optional<StringHash> meta_any_to_string_hash(const entt::meta_any& value);

	// Compares `left` and `right` to see if they both have the same string hash.
	bool meta_any_string_compare(const entt::meta_any& left, const entt::meta_any& right);

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

    // Returns true if the `value` specified has a reflected indirection function.
    bool value_has_indirection(const MetaAny& value, bool bypass_indirect_meta_any=false);

    // Returns true if the `type` specified has a reflected indirection function.
    bool type_has_indirection(const MetaType& type);

    // Returns true if the the type referenced by `type_id` has a reflected indirection function.
    bool type_has_indirection(MetaTypeID type_id);

    // Returns a new `MetaAny` instance if `value` could supply an enclosed (indirect) value.
    // NOTE: Recursion.
    MetaAny try_get_underlying_value(const MetaAny& value);

    // Returns a new `MetaAny` instance if `value` could supply an enclosed (indirect) value.
    MetaAny try_get_underlying_value(const MetaAny& value, Registry& registry, Entity entity);

    // Returns a new `MetaAny` instance if `value` could supply an enclosed (indirect) value.
    MetaAny try_get_underlying_value(const MetaAny& value, Registry& registry, Entity entity, const MetaEvaluationContext& context);

    // TODO: Implement standalone overload for `MetaEvaluationContext`.
    //MetaAny try_get_underlying_value(MetaAny& expr, const MetaEvaluationContext& context);

    // Attempts to invoke `function` with the `args` specified.
    // 
    // If invocation fails, the next overload of `function` will be attempted.
    // If all overloads fail, this function will return an empty `MetaAny` object.
    // 
    // NOTE: This function encapsulates `args` via `entt::forward_as_meta` automatically.
	template <typename InstanceType, typename ...Args>
	MetaAny invoke_any_overload(MetaFunction function, InstanceType&& instance, Args&&... args)
	{
		static_assert(std::is_same_v<std::decay_t<InstanceType>, MetaAny>, "The `instance` parameter must be a `MetaAny` object, or reference to one.");

		while (function)
		{
			// NOTE: Function `arity` check added to avoid unnecessary object creation overhead.
			if (function.arity() == sizeof...(args))
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

	template <typename InstanceType, typename ...Args>
	MetaAny invoke_any_overload(MetaFunction function, InstanceType&& instance, MetaAny* const args, std::size_t arg_count)
	{
		static_assert(std::is_same_v<std::decay_t<InstanceType>, MetaAny>, "The `instance` parameter must be a `MetaAny` object, or reference to one.");

		while (function)
		{
			// NOTE: We don't bother with the function `arity` check here,
			// since EnTT already handles that. (+ Copying a pointer is essentially free)
			if (auto result = function.invoke(instance, args, arg_count))
			{
				return result;
			}

			function = function.next();
		}

		return {};
	}

    // Attempts to retrieve an inner `MetaType` from `type_reference_value`, if it holds one (or the ID of one) internally.
    MetaType try_get_underlying_type(const MetaAny& type_reference_value);

    // Attempts to retrieve an inner `MetaType` from `direct_or_indirect_value`.
    // This routine will fallback to the type of `direct_or_indirect_value` if
    // it does not have an inner type, nor indirection.
    // 
    // See also: `type_has_indirection`, `value_has_indirection`
    MetaType get_underlying_or_direct_type(const MetaAny& direct_or_indirect_value);

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
    inline bool set_data_member(entt::meta_any& instance, std::string_view member_name, T&& member_value)
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

    // Returns a new `MetaAny` instance if `accessor` could supply an (indirect) value using `from`.
    template
    <
        typename AccessorType,
        typename ValueType,
        typename ...Args
    >
    std::enable_if_t
    <
        (std::is_same_v<std::decay_t<AccessorType>, MetaAny> && !std::is_same_v<std::decay_t<ValueType>, Registry>),
        MetaAny
    >
    try_get_underlying_value(AccessorType&& accessor, ValueType&& from, Args&&... args)
    {
        using namespace engine::literals;

        if (!accessor)
        {
            return {};
        }

        auto type = accessor.type();

        // NOTE: Since we use universal references for the `accessor` and `from` parameters,
        // we can assume they'll never be pure value types (as opposed to references).
        // Hence why we don't need additional logic for `is_const`.
        constexpr bool accessor_is_const = std::is_const_v<std::remove_reference_t<AccessorType>>;
        constexpr bool from_is_const = std::is_const_v<std::remove_reference_t<ValueType>>;

        constexpr bool is_const = (accessor_is_const || from_is_const);

        // NOTE: Possible recursion.
        if constexpr (std::is_same_v<std::decay_t<ValueType>, MetaAny>)
        {
            if (!from)
            {
                return {};
            }

            auto invoke_exact_ex = [&](auto&& fn, auto&& from_rep, auto&& pred, bool forward_from_as_meta=true) -> MetaAny
            {
                return util::drop_last_until_success<2>
                (
                    [&](auto&&... args_in) -> MetaAny
                    {
                        auto result = MetaAny {};
                        
                        auto current_fn = fn;

                        while (current_fn)
                        {
                            if (pred(current_fn))
                            {
                                if (forward_from_as_meta)
                                {
                                    /*
                                    std::array<MetaAny, (1 + sizeof...(args_in))> arguments =
                                    {
                                        MetaAnyWrapper { from_rep.as_ref() }, // entt::forward_as_meta
                                        entt::forward_as_meta<decltype(args_in)>(args_in)...
                                    };

                                    result = current_fn.invoke
                                    (
                                        std::forward<AccessorType>(accessor),
                                        static_cast<MetaAny* const>(arguments.data()),
                                        arguments.size()
                                    );
                                    */

                                    result = current_fn.invoke
                                    (
                                        std::forward<AccessorType>(accessor),
                                        entt::forward_as_meta(from_rep), // from_rep.as_ref()
                                        entt::forward_as_meta<decltype(args_in)>(args_in)...
                                    );
                                }
                                else
                                {
                                    result = current_fn.invoke
                                    (
                                        std::forward<AccessorType>(accessor),
                                        std::forward<decltype(from_rep)>(from_rep),
                                        entt::forward_as_meta<decltype(args_in)>(args_in)...
                                    );
                                }

                                if (result)
                                {
                                    break;
                                }
                            }

                            current_fn = current_fn.next();
                        }

                        return result;
                    },

                    args... // std::forward<Args>(args)...
                );
            };

            auto invoke_exact = [&invoke_exact_ex](auto&& fn, auto&& from_rep) -> MetaAny
            {
                return invoke_exact_ex
                (
                    std::forward<decltype(fn)>(fn),
                    std::forward<decltype(from_rep)>(from_rep),
                    [](auto&& current_fn) { return true; },
                    true
                );
            };

            auto from_ref = from.as_ref();

            auto get_fn_exact = MetaFunction {};

            if constexpr (sizeof...(args) > 0)
            {
                get_fn_exact = type.func("operator()"_hs);

                // Attempt to use a direct reference to the underlying object of `from` (with contextual arguments):
                if (get_fn_exact)
                {
                    auto result = invoke_exact_ex
                    (
                        get_fn_exact,
                        from_ref,
                        
                        [&from](auto&& current_fn) -> bool
                        {
                            if (current_fn.arity() == 0)
                            {
                                return false;
                            }

                            const auto first_argument_type = current_fn.arg(0);
                            const auto first_argument_type_id = first_argument_type.id();

                            switch (first_argument_type_id)
                            {
                                case entt::type_hash<MetaAny>::value():
                                case entt::type_hash<Registry>::value():
                                case entt::type_hash<MetaEvaluationContext>::value():
                                    break;

                                default:
                                    if (const auto from_type = from.type())
                                    {
                                        if (from_type.id() != entt::type_hash<MetaAny>::value())
                                        {
                                            return true;
                                        }
                                    }
                            }

                            return false;
                        },

                        false
                    );

                    if (result)
                    {
                        return result;
                    }
                }
            }

            if constexpr (sizeof...(args) > 0)
            {
                // Attempt to use an opaque reference to the underlying object of `from` (with contextual arguments):
                if (get_fn_exact)
                {
                    if (auto result = invoke_exact(get_fn_exact, from_ref))
                    {
                        return result;
                    }
                }
            }

            // Attempt to use an opaque reference to the underlying object of `from` (without contextual arguments):
            if (auto get_fn = type.func("operator()"_hs))
            {
                do
                {
                    if (auto result = get_fn.invoke(accessor, entt::forward_as_meta(from_ref)))
                    {
                        return result;
                    }

                    get_fn = get_fn.next();
                } while (get_fn);
            }

            if constexpr (!is_const)
            {
                if (auto set_fn = type.func("operator="_hs))
                {
                    do
                    {
                        if (auto result = set_fn.invoke(accessor, entt::forward_as_meta(from_ref)))
                        {
                            return result;
                        }

                        set_fn = set_fn.next();
                    } while (set_fn);
                }
            }

            if constexpr (sizeof...(Args) > 0)
            {
                // If this is a 'const' evaluation scenario, we can attempt to
                // fall back to a different function-argument configuration:
                if constexpr (is_const) // accessor_is_const
                {
                    // Check against the first trailing argument (after `from`) to see if it's a `MetaAny`:
                    if constexpr (std::is_same_v<std::decay<util::get_first_type<Args...>>, MetaAny>)
                    {
                        // Since the first trailing argument was a `MetaAny`
                        // (normally used in a non-const scenario as a 'destination'/'out' parameter),
                        // 
                        // and we already know we can't make use of that in this 'const' scenario,
                        // we're free to truncate the first argument and attempt evaluation again.
                        return util::drop_first
                        (
                            [&accessor, &from](auto&&... reduced_args)
                            {
                                // NOTE: Recursion.
                                // (Both from re-calling this implementation, and potentially calling a different implementation; see below)
                                return try_get_underlying_value
                                (
                                    std::forward<AccessorType>(accessor),
                                    std::forward<ValueType>(from),
                                    std::forward<decltype(reduced_args)>(reduced_args)...
                                );
                            },

                            std::forward<Args>(args)...
                        );
                    }
                    else
                    {
                        // NOTE: Since `from` is only used for added context when the evaluation is 'const',
                        // 
                        // and the first trailing argument passed does not contain a 'destination' field (see above),
                        // we are free to truncate that field as a fallback control-path.
                        // 
                        // NOTE: Rather than traditional recursion, this control-path is likely to hit a different implementation.
                        return try_get_underlying_value(std::forward<AccessorType>(accessor), std::forward<Args>(args)...);
                    }
                }
                else
                {
                    if (auto set_fn_exact = type.func("operator="_hs))
                    {
                        if (auto result = invoke_exact(set_fn_exact, from_ref))
                        {
                            return result;
                        }
                    }

                    return {};
                }
            }
        }
        else
        {
            auto from_ref = entt::forward_as_meta<ValueType>(from);

            if constexpr (is_const)
            {
                // Regular 'getter' behavior:
                return try_get_underlying_value
                (
                    std::forward<AccessorType>(accessor),
                    static_cast<const MetaAny&>(from_ref),
                    std::forward<Args>(args)...
                );
            }
            else
            {
                // 'Setter' behavior:
                return try_get_underlying_value
                (
                    std::forward<AccessorType>(accessor),
                    static_cast<MetaAny&>(from_ref),
                    std::forward<Args>(args)...
                );
            }
            
        }

        return {};
    }

    template <typename ValueType, typename ...Args>
    MetaAny get_indirect_value_or_copy(ValueType&& value, Args&&... args)
    {
        static_assert(std::is_same_v<std::decay_t<ValueType>, MetaAny>, "Argument `value` must be of type `MetaAny`.");

        // NOTE: We purposefully don't forward `value` here, since `value` is used
        // as the return value if an underlying value couldn't be resolved.
        if (auto indirect = try_get_underlying_value(value, std::forward<Args>(args)...))
        {
            return indirect;
        }

        return value;
    }

    template <typename ValueType, typename ...Args>
    MetaAny get_indirect_value_or_ref(ValueType&& value, Args&&... args)
    {
        static_assert(std::is_same_v<std::decay_t<ValueType>, MetaAny>, "Argument `value` must be of type `MetaAny`.");

        // NOTE: We purposefully don't forward `value` here, since a reference
        // to `value` is used if an underlying value couldn't be resolved.
        if (auto indirect = try_get_underlying_value(value, std::forward<Args>(args)...))
        {
            return indirect;
        }

        return value.as_ref();
    }

    template <typename FunctionType, typename ...Args>
    MetaAny execute_opaque_function(FunctionType&& opaque_function, Args&&... args)
    {
        static_assert(std::is_same_v<std::decay_t<FunctionType>, MetaAny>, "Argument `opaque_function` must be of type `MetaAny`.");

        return try_get_underlying_value(opaque_function, std::forward<Args>(args)...);
    }

    template <typename ExprType, typename ...Args>
    MetaAny execute_opaque_expression(ExprType&& opaque_expr, Args&&... args)
    {
        static_assert(std::is_same_v<std::decay_t<ExprType>, MetaAny>, "Argument `opaque_expr` must be of type `MetaAny`.");

        return try_get_underlying_value(opaque_expr, std::forward<Args>(args)...);
    }

	// Attempts to retrieve an instance of `Type` from `value`, passing it to `callback` if successful.
	template <typename Type, typename Callback>
	inline bool try_value(const entt::meta_any& value, Callback&& callback)
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
	inline bool try_string_value(const entt::meta_any& value, Callback&& callback)
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

    template <typename TypeReference>
	MetaType try_get_underlying_type(const TypeReference& exact_type_reference_value)
	{
		using reference_t = std::decay_t<TypeReference>;

        if constexpr (has_method_has_type_v<reference_t, bool>)
        {
            if (!exact_type_reference_value.has_type())
            {
                return {};
            }
        }

        // Specialization added due to extra levels of indirection:
		if constexpr (std::is_same_v<reference_t, MetaFunctionCall>)
		{
			if (auto target_fn = exact_type_reference_value.get_function())
			{
				if (auto return_type = target_fn.ret())
				{
					return return_type;
				}
			}
		}
        else if constexpr (std::is_same_v<reference_t, MetaDataMember> || std::is_same_v<reference_t, IndirectMetaDataMember>)
        {
            return exact_type_reference_value.get_member_type();
        }
		else
		{
            if constexpr (has_method_get_type_v<reference_t, MetaType>)
            {
                return exact_type_reference_value.get_type();
            }
            else if constexpr (has_method_get_type_v<reference_t, MetaTypeID>)
            {
                const auto type_id = exact_type_reference_value.get_type();

                return resolve(type_id);
            }
		}

		return {};
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