#pragma once

#include "function.hpp"

#include <engine/types.hpp>

#include <engine/meta/types.hpp>
#include <engine/meta/traits.hpp>
#include <engine/meta/hash.hpp>
//#include <engine/meta/meta_evaluation_context.hpp>

#include <entt/meta/meta.hpp>
#include <entt/meta/policy.hpp>

#include <utility>
#include <type_traits>
#include <tuple>

namespace engine
{
    struct MetaEvaluationContext;

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
        using namespace engine::literals;

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
        using namespace engine::literals;

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
}