#pragma once

#include <type_traits>
//#include <utility>

// Function traits:

// This macro generates a trait that checks if a type has a static member function with a given signature.
#define GENERATE_HAS_FUNCTION_TRAIT(function_name) \
    GENERATE_HAS_FUNCTION_TRAIT_EX(has_function_##function_name, function_name)

#define GENERATE_HAS_FUNCTION_TRAIT_EX(trait_name, function_name)                                                                    \
    template <typename, typename T>                                                                                                  \
    struct trait_name                                                                                                                \
    {                                                                                                                                \
        static_assert                                                                                                                \
        (                                                                                                                            \
            std::integral_constant<T, false>::value,                                                                                 \
            "The second template parameter needs to be a function type."                                                             \
        );                                                                                                                           \
    };                                                                                                                               \
                                                                                                                                     \
    template <typename Type, typename ReturnType, typename... Args>                                                                  \
    struct trait_name<Type, ReturnType(Args...)>                                                                                     \
    {                                                                                                                                \
        private:                                                                                                                     \
            template<typename T, T> struct type_and_signature;                                                                       \
            template<typename T> static std::true_type  has_function(type_and_signature<ReturnType(Args...), (&T::function_name)>*); \
            template<typename T> static std::false_type has_function(...);                                                           \
        public:                                                                                                                      \
            static constexpr bool value = std::is_same_v<decltype(has_function<Type>(nullptr)), std::true_type>;                     \
            static constexpr auto ptr()                                                                                              \
            {                                                                                                                        \
                if constexpr (value)                                                                                                 \
                {                                                                                                                    \
                    return static_cast<ReturnType(Args...)>((&Type::function_name));                                                 \
                }                                                                                                                    \
                else                                                                                                                 \
                {                                                                                                                    \
                    return nullptr;                                                                                                  \
                }                                                                                                                    \
            }                                                                                                                        \
    };                                                                                                                               \
                                                                                                                                     \
    template <typename Type, typename ReturnType, typename... Args>                                                                  \
    inline constexpr bool trait_name##_v = trait_name<Type, ReturnType, Args...>::value;

// Shorthand for using a generated trait from `GENERATE_HAS_FUNCTION_TRAIT`.
#define TYPE_HAS_FUNCTION(type, function_name, function_signature) \
    has_function_##function_name<type, function_signature>::value

// This macro generates an alias to a 'has_function' trait for a given function signature.
// See also: `GENERATE_HAS_FUNCTION_TRAIT`, `GENERATE_EXACT_FUNCTION_TRAIT`
#define GENERATE_HAS_FUNCTION_TRAIT_SPECIALIZATION(specialization_name, function_name, function_signature) \
    template <typename Type>                                                                               \
    using specialization_name##_t = has_function_##function_name<Type, function_signature>;                \
                                                                                                           \
    template <typename Type>                                                                               \
    constexpr bool specialization_name() { return specialization_name##_t<Type>::value; } // static

// This generates a 'has_function' trait then generates a specialization alias for a given signature.
// Equivalent to calling `GENERATE_HAS_FUNCTION_TRAIT` then `GENERATE_HAS_FUNCTION_TRAIT_SPECIALIZATION`.
#define GENERATE_EXACT_FUNCTION_TRAIT(specialization_name, function_name, function_signature)           \
    GENERATE_HAS_FUNCTION_TRAIT(function_name);                                                         \
    GENERATE_HAS_FUNCTION_TRAIT_SPECIALIZATION(specialization_name, function_name, function_signature);

// Same behavior as `GENERATE_EXACT_FUNCTION_TRAIT`, but defaults the `specialization_name` to `has_[function_name]`.
#define GENERATE_EXACT_FUNCTION_TRAIT_SIMPLIFIED(function_name, function_signature) \
    GENERATE_EXACT_FUNCTION_TRAIT(has_##function_name, function_name, function_signature);


// Method traits:

// This macro generates a trait that checks if a type has a non-static member-function (method) with a given signature.
#define GENERATE_HAS_METHOD_TRAIT(method_name) \
    GENERATE_HAS_METHOD_TRAIT_EX(has_method_##method_name, method_name)

#define GENERATE_HAS_METHOD_TRAIT_EX(trait_name, method_name)                                                      \
    template <typename, typename T>                                                                                \
    struct trait_name                                                                                              \
    {                                                                                                              \
        static_assert                                                                                              \
        (                                                                                                          \
            std::integral_constant<T, false>::value,                                                               \
            "The second template parameter must be a function type."                                               \
        );                                                                                                         \
    };                                                                                                             \
                                                                                                                   \
    template <typename Type, typename ReturnType, typename... Args>                                                \
    struct trait_name<Type, ReturnType(Args...)>                                                                   \
    {                                                                                                              \
        private:                                                                                                   \
            template <typename T>                                                                                  \
            static constexpr auto check(T*)                                                                        \
            -> typename std::is_same                                                                               \
            <                                                                                                      \
                decltype(std::declval<T>().method_name(std::declval<Args>()...)),                                  \
                ReturnType                                                                                         \
            >::type;                                                                                               \
                                                                                                                   \
             template <typename>                                                                                   \
            static constexpr std::false_type check(...);                                                           \
                                                                                                                   \
            using type = decltype(check<Type>(nullptr));                                                           \
        public:                                                                                                    \
            static constexpr bool value = type::value;                                                             \
                                                                                                                   \
            template <bool is_const=false, bool is_noexcept=false>                                                 \
            static constexpr auto ptr()                                                                            \
            {                                                                                                      \
                if constexpr (value)                                                                               \
                {                                                                                                  \
                    if constexpr (is_const)                                                                        \
                    {                                                                                              \
                        if constexpr (is_noexcept)                                                                 \
                        {                                                                                          \
                            return static_cast<ReturnType(Type::*)(Args...) const noexcept>((&Type::method_name)); \
                        }                                                                                          \
                        else                                                                                       \
                        {                                                                                          \
                            return static_cast<ReturnType(Type::*)(Args...) const>((&Type::method_name));          \
                        }                                                                                          \
                    }                                                                                              \
                    else                                                                                           \
                    {                                                                                              \
                        if constexpr (is_noexcept)                                                                 \
                        {                                                                                          \
                            return static_cast<ReturnType(Type::*)(Args...) noexcept>((&Type::method_name));       \
                        }                                                                                          \
                        else                                                                                       \
                        {                                                                                          \
                            return static_cast<ReturnType(Type::*)(Args...)>((&Type::method_name));                \
                        }                                                                                          \
                    }                                                                                              \
                }                                                                                                  \
                else                                                                                               \
                {                                                                                                  \
                    return nullptr;                                                                                \
                }                                                                                                  \
            }                                                                                                      \
    };                                                                                                             \
                                                                                                                   \
    template <typename Type, typename ReturnType, typename... Args>                                                \
    inline constexpr bool trait_name##_v = trait_name<Type, ReturnType(Args...)>::value;

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
	template <typename T, typename = int>                                                   \
	struct has_field_##field_name : std::false_type {};                                     \
                                                                                            \
	template <typename T>                                                                   \
	struct has_field_##field_name<T, decltype((void)T::field_name, 0)> : std::true_type {}; \
	                                                                                        \
	template <typename T>                                                                   \
	constexpr bool has_field_##field_name##_v = has_field_##field_name<T>::value;