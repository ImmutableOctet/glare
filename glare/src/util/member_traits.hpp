#pragma once

#include <type_traits>
//#include <utility>

// Function traits:

// This macro generates a trait that checks if a type has a static member function with a given signature.
#define GENERATE_HAS_FUNCTION_TRAIT(function_name) \
    GENERATE_HAS_FUNCTION_TRAIT_EX(has_function_##function_name, function_name)

#define GENERATE_HAS_FUNCTION_TRAIT_EX(trait_name, function_name)                                                                    \
    template <typename Type, typename ReturnType, typename... Args>                                                                  \
    struct trait_name                                                                                                                \
    {                                                                                                                                \
        private:                                                                                                                     \
            template<typename T, T> struct type_and_signature;                                                                       \
            template<typename T> static std::true_type  has_function(type_and_signature<ReturnType(Args...), (&T::function_name)>*); \
            template<typename T> static std::false_type has_function(...);                                                           \
                                                                                                                                     \
            template <bool is_noexcept=false>                                                                                        \
            static constexpr auto get_ptr()                                                                                          \
            {                                                                                                                        \
                if constexpr (value)                                                                                                 \
                {                                                                                                                    \
                    if constexpr (is_noexcept)                                                                                       \
                    {                                                                                                                \
                        return static_cast<ReturnType(Args...) noexcept>((&Type::function_name));                                    \
                    }                                                                                                                \
                    else                                                                                                             \
                    {                                                                                                                \
                        return static_cast<ReturnType(Args...)>((&Type::function_name));                                             \
                    }                                                                                                                \
                }                                                                                                                    \
                else                                                                                                                 \
                {                                                                                                                    \
                    return nullptr;                                                                                                  \
                }                                                                                                                    \
            }                                                                                                                        \
        public:                                                                                                                      \
            inline static constexpr bool value = std::is_same_v<decltype(has_function<Type>(nullptr)), std::true_type>;              \
                                                                                                                                     \
            template <bool is_noexcept=false>                                                                                        \
            static constexpr auto ptr = get_ptr<is_noexcept>();                                                                      \
    };                                                                                                                               \
                                                                                                                                     \
    template <typename Type, typename ReturnType, typename... Args>                                                                  \
    inline constexpr bool trait_name##_v = trait_name<Type, ReturnType, Args...>::value;

// Shorthand for using a generated trait from `GENERATE_HAS_FUNCTION_TRAIT`.
#define TYPE_HAS_FUNCTION(type, function_name, function_signature) \
    has_function_##function_name<type, function_signature>::value

// This macro generates an alias to a 'has_function' trait for a given function signature.
// See also: `GENERATE_HAS_FUNCTION_TRAIT`, `GENERATE_EXACT_FUNCTION_TRAIT`
#define GENERATE_HAS_FUNCTION_TRAIT_SPECIALIZATION(specialization_name, function_name, ...)         \
    template <typename Type>                                                                        \
    using specialization_name##_t = has_function_##function_name<Type, __VA_ARGS__>;                \
                                                                                                    \
    template <typename Type>                                                                        \
    constexpr bool specialization_name() { return specialization_name##_t<Type>::value; } // static

// This generates a 'has_function' trait then generates a specialization alias for a given signature.
// Equivalent to calling `GENERATE_HAS_FUNCTION_TRAIT` then `GENERATE_HAS_FUNCTION_TRAIT_SPECIALIZATION`.
#define GENERATE_EXACT_FUNCTION_TRAIT(specialization_name, function_name, ...)                   \
    GENERATE_HAS_FUNCTION_TRAIT(function_name);                                                  \
    GENERATE_HAS_FUNCTION_TRAIT_SPECIALIZATION(specialization_name, function_name, __VA_ARGS__);

// Same behavior as `GENERATE_EXACT_FUNCTION_TRAIT`, but defaults the `specialization_name` to `has_[function_name]`.
#define GENERATE_EXACT_FUNCTION_TRAIT_SIMPLIFIED(function_name, ...) \
    GENERATE_EXACT_FUNCTION_TRAIT(has_##function_name, function_name, __VA_ARGS__);


// Method traits:

// This macro generates a trait that checks if a type has a non-static member-function (method) with a given signature.
#define GENERATE_HAS_METHOD_TRAIT(method_name) \
    GENERATE_HAS_METHOD_TRAIT_EX(has_method_##method_name, method_name)

#define GENERATE_HAS_METHOD_TRAIT_EX(trait_name, method_name)                                                      \
    template <typename Type, typename ReturnType, typename... Args>                                                \
    struct trait_name                                                                                              \
    {                                                                                                              \
        private:                                                                                                   \
            template <typename, typename=void>                                                                     \
            struct check : std::false_type {};                                                                     \
                                                                                                                   \
            template <typename T>                                                                                  \
            struct check                                                                                           \
            <                                                                                                      \
                T,                                                                                                 \
                typename std::enable_if                                                                            \
                <                                                                                                  \
                    std::is_same                                                                                   \
                    <                                                                                              \
                        decltype(std::declval<T>().method_name(std::declval<Args>()...)),                          \
                        ReturnType                                                                                 \
                    >::value                                                                                       \
                >::type                                                                                            \
            > : std::true_type {};                                                                                 \
                                                                                                                   \
            template <bool is_const=false, bool is_noexcept=false>                                                 \
            using member_function_type = std::conditional                                                          \
            <                                                                                                      \
                is_const,                                                                                          \
                                                                                                                   \
                typename std::conditional                                                                          \
                <                                                                                                  \
                    is_noexcept,                                                                                   \
                                                                                                                   \
                    ReturnType(Type::*)(Args...) const noexcept,                                                   \
                    ReturnType(Type::*)(Args...) const                                                             \
                >::type,                                                                                           \
                                                                                                                   \
                typename std::conditional                                                                          \
                <                                                                                                  \
                    is_noexcept,                                                                                   \
                                                                                                                   \
                    ReturnType(Type::*)(Args...) noexcept,                                                         \
                    ReturnType(Type::*)(Args...)                                                                   \
                >::type                                                                                            \
            >::type;                                                                                               \
                                                                                                                   \
            template <typename trait_check, bool is_const=false, bool is_noexcept=false, typename=void>            \
            struct get_ptr_t                                                                                       \
            {                                                                                                      \
                static constexpr auto ptr() { return nullptr; }                                                    \
            };                                                                                                     \
                                                                                                                   \
            template <typename trait_check, bool is_const, bool is_noexcept>                                       \
            struct get_ptr_t                                                                                       \
            <                                                                                                      \
                trait_check,                                                                                       \
                is_const, is_noexcept,                                                                             \
                                                                                                                   \
                /*                                                                                                 \
                std::enable_if_t                                                                                   \
                <                                                                                                  \
                    trait_check::value,                                                                            \
                                                                                                                   \
                    std::void_t                                                                                    \
                    <                                                                                              \
                        decltype(static_cast<member_function_type<is_const, is_noexcept>>(&Type::method_name))     \
                    >                                                                                              \
                >                                                                                                  \
                */                                                                                                 \
                                                                                                                   \
                std::void_t                                                                                        \
                <                                                                                                  \
                    decltype(static_cast<member_function_type<is_const, is_noexcept>>(&Type::method_name))         \
                >                                                                                                  \
            >                                                                                                      \
            {                                                                                                      \
                static constexpr auto ptr()                                                                        \
                {                                                                                                  \
                    return static_cast<member_function_type<is_const, is_noexcept>>(&Type::method_name);           \
                }                                                                                                  \
            };                                                                                                     \
        public:                                                                                                    \
            inline static constexpr bool value = check<Type>::value;                                               \
                                                                                                                   \
            template <bool is_const=false, bool is_noexcept=false>                                                 \
            inline static constexpr auto ptr = get_ptr_t<check<Type>, is_const, is_noexcept>::ptr();               \
    };                                                                                                             \
                                                                                                                   \
    template <typename Type, typename ReturnType, typename... Args>                                                \
    inline constexpr bool trait_name##_v = trait_name<Type, ReturnType, Args...>::value;

// Shorthand for using a generated trait from `GENERATE_HAS_METHOD_TRAIT`.
#define TYPE_HAS_METHOD(type, method_name, function_signature) \
    has_method_##method_name<type, function_signature>::value

// This macro generates an alias to a 'has_method' trait for a given function signature.
// See also: `GENERATE_HAS_METHOD_TRAIT`, `GENERATE_EXACT_METHOD_TRAIT`
#define GENERATE_HAS_METHOD_TRAIT_SPECIALIZATION(specialization_name, method_name, function_signature) \
    template <typename Type>                                                                           \
    using specialization_name##_t = has_method_##method_name<Type, function_signature>;                \
                                                                                                       \
    template <typename Type>                                                                           \
    constexpr bool specialization_name() { return specialization_name_t<Type>::value; } // static
    


// This generates a 'has_method' trait then generates a specialization alias for a given signature.
// Equivalent to calling `GENERATE_HAS_METHOD_TRAIT` then `GENERATE_HAS_METHOD_TRAIT_SPECIALIZATION`.
#define GENERATE_EXACT_METHOD_TRAIT(specialization_name, method_name, function_signature)           \
    GENERATE_HAS_METHOD_TRAIT(method_name);                                                         \
    GENERATE_HAS_METHOD_TRAIT_SPECIALIZATION(specialization_name, method_name, function_signature);

// Same behavior as `GENERATE_EXACT_METHOD_TRAIT`, but defaults the `specialization_name` to `has_[method_name]`.
#define GENERATE_EXACT_METHOD_TRAIT_SIMPLIFIED(method_name, function_signature) \
    GENERATE_EXACT_METHOD_TRAIT(has_##method_name, method_name, function_signature);

#define GENERATE_HAS_FIELD_TRAIT(field_name)                                                \
	template <typename T, typename=int>                                                     \
	struct has_field_##field_name : std::false_type {};                                     \
                                                                                            \
	template <typename T>                                                                   \
	struct has_field_##field_name<T, decltype((void)T::field_name, 0)> : std::true_type {}; \
	                                                                                        \
	template <typename T>                                                                   \
	inline constexpr bool has_field_##field_name##_v = has_field_##field_name<T>::value;