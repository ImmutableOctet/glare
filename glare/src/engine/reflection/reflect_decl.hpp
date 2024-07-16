#pragma once

#define GLARE_IMPL_DECLARE_REFLECT() \
    template <typename T=void>       \
    void reflect();

#define GLARE_IMPL_DECLARE_REFLECTED_TYPE(Type) \
    template <> void reflect<Type>();

#define GLARE_IMPL_DECLARE_REFLECTED_TYPE_NAMESPACED(Namespace, Type) \
    template <> void ::Namespace::reflect<Type>();

#define GLARE_IMPL_CALL_REFLECT(Type) \
	::engine::reflect<Type>();

#define GLARE_IMPL_DEFINE_REFLECTION_FORWARDING_EX(Type, REFLECT_FN) \
    template <>                                       \
    void reflect<Type>()                              \
    {                                                 \
        REFLECT_FN(Type);                             \
    }

#define GLARE_IMPL_DEFINE_REFLECTION_FORWARDING(Type) \
	GLARE_IMPL_DEFINE_REFLECTION_FORWARDING_EX(Type, GLARE_IMPL_CALL_REFLECT)

namespace engine
{
    // NOTE: In the default case of `T=void`, the overridden version of this template is used.
    // TODO: Look into best way to handle multiple calls to reflect. (This is currently only managed in `reflect_all`)
    GLARE_IMPL_DECLARE_REFLECT();

    // Aliases the default configuration of `reflect` to the `reflect_all` free-function.
    template <> void reflect<void>();
}