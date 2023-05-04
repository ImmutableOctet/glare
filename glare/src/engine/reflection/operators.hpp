#pragma once

#include "common_extensions.hpp"

#include <engine/meta/traits.hpp>
#include <engine/meta/types.hpp>
#include <engine/meta/hash.hpp>

#include <util/type_traits.hpp>

#include <entt/meta/meta.hpp>

#include <utility>
#include <type_traits>

namespace engine
{
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
        using namespace engine::literals;

        constexpr bool has_equality_operator = has_meta_operator_method<has_method_operator_equal, T, bool, OtherType, method_is_const, method_is_noexcept>();
        constexpr bool has_inequality_operator = has_meta_operator_method<has_method_operator_not_equal, T, bool, OtherType, method_is_const, method_is_noexcept>();

        if constexpr (has_equality_operator)
        {
            type = define_meta_operator_method<has_method_operator_equal, T, bool, OtherType>(type, "operator=="_hs);

            if constexpr (!has_inequality_operator)
            {
                type = type.template func<&impl::inequality_operator_fallback_impl<T, OtherType>>("operator!="_hs);
            }
        }
        
        if constexpr (has_inequality_operator)
        {
            type = define_meta_operator_method<has_method_operator_not_equal, T, bool, OtherType>(type, "operator!="_hs);

            if constexpr (!has_equality_operator)
            {
                type = type.template func<&impl::equality_operator_fallback_impl<T, OtherType>>("operator=="_hs);
            }
        }

        return type;
    }

    template <typename T>
    auto define_boolean_operators(auto type)
    {
        using namespace engine::literals;

        // Check for contextual conversion to boolean.
        if constexpr (util::is_convertible_to_bool<T>)
        {
            return type
                .func<&impl::operator_bool_impl<T>>("operator bool"_hs)
                .func<&impl::operator_logical_not_impl<T>>("!operator"_hs)
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
}