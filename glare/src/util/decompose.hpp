#pragma once

#include <type_traits>
#include <utility>
#include <tuple>

namespace util
{
	namespace impl
    {
        template <auto member_function_ptr, bool ptr_is_noexcept, typename ReturnType, typename Type, typename TypeReference, typename... Args>
        struct decompose_member_function_impl_shared
        {
            using self_type            = Type;
            using function_type        = ReturnType(*)(Args...);
            using return_type          = ReturnType;
            using args                 = std::tuple<Args...>;
            using self_reference_type  = TypeReference;

            inline static constexpr std::size_t argument_count = sizeof...(Args);

            inline static constexpr bool is_noexcept           = ptr_is_noexcept;
            inline static constexpr bool is_const              = std::is_const_v<std::remove_reference_t<TypeReference>>;

            using member_function_type_nonconst = ReturnType(Type::*)(Args...);
            using member_function_type_nonconst_noexcept = ReturnType(Type::*)(Args...) noexcept;

            using member_function_type_const = ReturnType(Type::*)(Args...) const;
            using member_function_type_const_noexcept = ReturnType(Type::*)(Args...) const noexcept;

            using member_function_type = std::conditional_t
            <
                is_const,

                std::conditional_t
                <
                    is_noexcept,
                    member_function_type_const_noexcept,
                    member_function_type_const
                >,

                std::conditional_t
                <
                    is_noexcept,
                    member_function_type_nonconst_noexcept,
                    member_function_type_nonconst
                >
            >;

            static constexpr ReturnType decomposed(TypeReference self, Args... args) noexcept(is_noexcept)
            {
                if constexpr (std::is_same_v<ReturnType, void>)
                {
                    (self.*member_function_ptr)(args...);
                }
                else
                {
                    return (self.*member_function_ptr)(args...);
                }
            }
        
            inline static constexpr auto decomposed_ptr = &decomposed;

            using decomposed_ptr_type = decltype(decomposed_ptr);
        };

        template <auto member_function_ptr, typename MemberFunctionType>
        struct decompose_member_function_impl;

        template <auto member_function_ptr, typename ReturnType, typename Type, typename ...Args>
        struct decompose_member_function_impl<member_function_ptr, ReturnType(Type::*)(Args...)>
            : decompose_member_function_impl_shared<member_function_ptr, false, ReturnType, Type, Type&, Args...>
        {};

        template <auto member_function_ptr, typename ReturnType, typename Type, typename ...Args>
        struct decompose_member_function_impl<member_function_ptr, ReturnType(Type::*)(Args...) noexcept>
            : decompose_member_function_impl_shared<member_function_ptr, true, ReturnType, Type, Type&, Args...>
        {};

        template <auto member_function_ptr, typename ReturnType, typename Type, typename ...Args>
        struct decompose_member_function_impl<member_function_ptr, ReturnType(Type::*)(Args...) const>
            : decompose_member_function_impl_shared<member_function_ptr, false, ReturnType, Type, const Type&, Args...>
        {};

        template <auto member_function_ptr, typename ReturnType, typename Type, typename ...Args>
        struct decompose_member_function_impl<member_function_ptr, ReturnType(Type::*)(Args...) const noexcept>
            : decompose_member_function_impl_shared<member_function_ptr, true, ReturnType, Type, const Type&, Args...>
        {};
    }

    template <auto member_function_ptr>
    using decompose_member_function = impl::decompose_member_function_impl<member_function_ptr, decltype(member_function_ptr)>;

    template <auto member_function_ptr>
    using function_type_from_member_function_signature = typename decompose_member_function<member_function_ptr>::function_type;
}