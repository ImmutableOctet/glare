#pragma once

#include "types.hpp"
#include "hash.hpp"
#include "traits.hpp"

#include <entt/meta/meta.hpp>
//#include <entt/entt.hpp>

#include <util/algorithm.hpp>

#include <type_traits>
#include <utility>

namespace engine
{
    struct MetaEvaluationContext;

    struct MetaFunctionCall;
    struct MetaTypeReference;
    struct MetaDataMember;
    struct IndirectMetaDataMember;

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
    MetaAny try_get_underlying_value(const MetaAny& value, const MetaEvaluationContext& context);

    // Returns a new `MetaAny` instance if `value` could supply an enclosed (indirect) value.
    MetaAny try_get_underlying_value(const MetaAny& value, Registry& registry, Entity entity, const MetaEvaluationContext& context);

    // Attempts to retrieve an inner `MetaType` from `type_reference_value`, if it holds one (or the ID of one) internally.
    MetaType try_get_underlying_type(const MetaAny& type_reference_value);

    // Attempts to retrieve an inner `MetaType` from `direct_or_indirect_value`.
    // This routine will fallback to the type of `direct_or_indirect_value` if
    // it does not have an inner type, nor indirection.
    // 
    // See also: `type_has_indirection`, `value_has_indirection`
    MetaType get_underlying_or_direct_type(const MetaAny& direct_or_indirect_value);

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
                                case entt::type_hash<MetaEvaluationContext>::value(): // "MetaEvaluationContext"_hs:
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
}