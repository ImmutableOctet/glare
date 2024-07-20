#pragma once

#include "script_entry_point.hpp"

#include <engine/types.hpp>

#include <util/type_algorithms.hpp>

#include <utility>
#include <type_traits>
#include <optional>

#define GLARE_SCRIPT_IMPL_VA_ARGS_EMPTY(...) \
    (sizeof(#__VA_ARGS__) == 1)

// Alternative implementation (does not work on MSVC):
//  #define GLARE_SCRIPT_IMPL_VA_ARGS_EMPTY(...) \
//      (sizeof((char[]){ #__VA_ARGS__ }) == 1)

#define GLARE_SCRIPT_IMPL_EMBEDDED_SCRIPT_IS_LAMBDA(...) \
    ((#__VA_ARGS__)[0] == '[')

#define GLARE_SCRIPT_LAMBDA_DECL(...) \
    [&](__VA_ARGS__) -> engine::ScriptFiber

#define GLARE_SCRIPT_LAMBDA_STANDARD_DECL() \
    GLARE_SCRIPT_LAMBDA_DECL(GLARE_SCRIPT_ENTRYPOINT_PARAMETER_LIST)

#define GLARE_SCRIPT_INLINE_EMBED_INSTANTIATE_SCRIPT_TYPE(embedded_script_t) \
    embedded_script_t { get_registry(), get_entity(), get_context(), engine::EntityThreadInterface::get_thread_name<embedded_script_t>() }

#define GLARE_SCRIPT_IMPL_GENERATE_EMBEDDED_SCRIPT_WRAPPER_TYPE_BEGIN(embedded_script_entry_point_wrapper_t, embedded_script_extension_type_t, BaseScriptType) \
    class embedded_script_entry_point_wrapper_t : public BaseScriptType, public embedded_script_extension_type_t                                               \
    {                                                                                                                                                          \
        public:                                                                                                                                                \
            using Base = BaseScriptType;                                                                                                                       \
		    using ScriptID = engine::StringHash;                                                                                                               \
            using CoreScriptType = typename BaseScriptType::CoreScriptType;                                                                                    \
                                                                                                                                                               \
            embedded_script_entry_point_wrapper_t                                                                                                              \
			(                                                                                                                                                  \
				engine::Registry& registry,                                                                                                                    \
				engine::Entity entity,                                                                                                                         \
				const engine::MetaEvaluationContext& context,                                                                                                  \
                                                                                                                                                               \
				ScriptID script_id={}                                                                                                                          \
			) :                                                                                                                                                \
				BaseScriptType(context, registry, script_id, entity)                                                                                           \
			{}                                                                                                                                                 \
                                                                                                                                                               \
            embedded_script_entry_point_wrapper_t(embedded_script_entry_point_wrapper_t&&) noexcept = default;                                                 \
            embedded_script_entry_point_wrapper_t(const embedded_script_entry_point_wrapper_t&) = delete;                                                      \
                                                                                                                                                               \
            embedded_script_entry_point_wrapper_t& operator=(embedded_script_entry_point_wrapper_t&&) noexcept = default;                                      \
            embedded_script_entry_point_wrapper_t& operator=(const embedded_script_entry_point_wrapper_t&) = delete;                                           \
                                                                                                                                                               \
            auto _generate_entry_point_lambda()                                                                                                                \
            {                                                                                                                                                  \
                using namespace glare_script_common;                                                                                                           \
                                                                                                                                                               \
                using namespace ::engine;                                                                                                                      \
                using namespace ::engine::script;
                
                // {Entry-point lambda return statement here}
                
#define GLARE_SCRIPT_IMPL_GENERATE_EMBEDDED_SCRIPT_WRAPPER_TYPE_END()                                                                                          \
            }                                                                                                                                                  \
    };

#define GLARE_SCRIPT_IMPL_GENERATE_EMBEDDED_SCRIPT_WRAPPER_TYPE(embedded_script_entry_point_wrapper_t, BaseScriptType, SCRIPT_BODY)                        \
    GLARE_SCRIPT_IMPL_GENERATE_EMBEDDED_SCRIPT_WRAPPER_TYPE_BEGIN(embedded_script_entry_point_wrapper_t, util::empty_type<BaseScriptType>, BaseScriptType) \
        return SCRIPT_BODY;                                                                                                                                \
    GLARE_SCRIPT_IMPL_GENERATE_EMBEDDED_SCRIPT_WRAPPER_TYPE_END()

#define GLARE_SCRIPT_IMPL_GENERATE_SCRIPT_TYPE(embedded_script_t, embedded_script_entry_point_wrapper_t, ...)        \
    class embedded_script_t : public embedded_script_entry_point_wrapper_t, public engine::impl::EmbeddedScriptMixin \
    {                                                                                                                \
        public:                                                                                                      \
            using Base           = embedded_script_entry_point_wrapper_t;                                            \
            using ScriptID       = Base::ScriptID;                                                                   \
            using WrapperType    = Base;                                                                             \
            using BaseScriptType = typename Base::Base;                                                              \
                                                                                                                     \
            using EntryPointType = decltype(std::declval<Base>()._generate_entry_point_lambda());                    \
			                                                                                                         \
			embedded_script_t                                                                                        \
			(                                                                                                        \
				engine::Registry& registry,                                                                          \
				engine::Entity entity,                                                                               \
				const engine::MetaEvaluationContext& context,                                                        \
					                                                                                                 \
				ScriptID script_id={}                                                                                \
			) :                                                                                                      \
				Base(registry, entity, context, script_id),                                                          \
                entry_point(std::nullopt)                                                                            \
			{}                                                                                                       \
			                                                                                                         \
            engine::ScriptFiber operator()()                                                                         \
            {                                                                                                        \
                if constexpr (GLARE_SCRIPT_IMPL_VA_ARGS_EMPTY(__VA_ARGS__))                                          \
                {                                                                                                    \
                    return BaseScriptType::operator()();                                                             \
                }                                                                                                    \
                else                                                                                                 \
                {                                                                                                    \
                    return {};                                                                                       \
                }                                                                                                    \
            }                                                                                                        \
                                                                                                                     \
            engine::ScriptFiber operator()(GLARE_SCRIPT_ENTRYPOINT_PARAMETER_LIST)                                   \
            {                                                                                                        \
                if constexpr (GLARE_SCRIPT_IMPL_VA_ARGS_EMPTY(__VA_ARGS__))                                          \
                {                                                                                                    \
                    return _script_body(GLARE_SCRIPT_ENTRYPOINT_PARAMETER_LIST_NAMES);                               \
                }                                                                                                    \
                else                                                                                                 \
                {                                                                                                    \
                    return {};                                                                                       \
                }                                                                                                    \
            }                                                                                                        \
                                                                                                                     \
            using engine::impl::EmbeddedScriptMixin::operator();                                                     \
			                                                                                                         \
				                                                                                                     \
            engine::ScriptFiber _script_body(GLARE_SCRIPT_ENTRYPOINT_PARAMETER_LIST __VA_ARGS__)                     \
            {                                                                                                        \
                                                                                                                     \
                if (!entry_point)                                                                                    \
                {                                                                                                    \
                    entry_point.emplace(Base::_generate_entry_point_lambda());                                       \
                }                                                                                                    \
                                                                                                                     \
                return util::drop_last_until_success<0>                                                              \
			    (                                                                                                    \
				    [&](auto&&... args) -> engine::ScriptFiber                                                       \
				    {                                                                                                \
					    if constexpr (std::is_invocable_r_v<engine::ScriptFiber, EntryPointType, decltype(args)...>) \
					    {                                                                                            \
						    return (*entry_point)(std::forward<decltype(args)>(args)...);                            \
					    }                                                                                            \
					    else                                                                                         \
					    {                                                                                            \
						    return ScriptFiber {};                                                                   \
					    }                                                                                            \
				    },                                                                                               \
				                                                                                                     \
                    GLARE_SCRIPT_ENTRYPOINT_PARAMETER_LIST_NAMES                                                     \
			    );                                                                                                   \
            }                                                                                                        \
        private:                                                                                                     \
            std::optional<EntryPointType> entry_point;                                                               \
    };

#define GLARE_SCRIPT_IMPL_GENERATE_EMBEDDED_SCRIPT_TYPE(embedded_script_t, BaseScriptType, SCRIPT_BODY, ...)                      \
    GLARE_SCRIPT_IMPL_GENERATE_EMBEDDED_SCRIPT_WRAPPER_TYPE(embedded_script_t##_entry_point_wrapper, BaseScriptType, SCRIPT_BODY) \
                                                                                                                                  \
    GLARE_SCRIPT_IMPL_GENERATE_SCRIPT_TYPE(embedded_script_t, embedded_script_t##_entry_point_wrapper, __VA_ARGS__)

#define GLARE_SCRIPT_LAMBDA_BEGIN_EX(embedded_script_entry_point_wrapper_t, embedded_script_extension_type_t, BaseScriptType)                                  \
    [&]()                                                                                                                                                      \
    {                                                                                                                                                          \
        GLARE_SCRIPT_IMPL_GENERATE_EMBEDDED_SCRIPT_WRAPPER_TYPE_BEGIN(embedded_script_entry_point_wrapper_t, embedded_script_extension_type_t, BaseScriptType) \
                                                                                                                                                               \
        return 
        
        // {Script lambda declaration and body here}

#define GLARE_SCRIPT_LAMBDA_END_EX(embedded_script_t, embedded_script_entry_point_wrapper_t, ...)                                                              \
        ;                                                                                                                                                      \
                                                                                                                                                               \
        GLARE_SCRIPT_IMPL_GENERATE_EMBEDDED_SCRIPT_WRAPPER_TYPE_END()                                                                                          \
                                                                                                                                                               \
        GLARE_SCRIPT_IMPL_GENERATE_SCRIPT_TYPE(embedded_script_t, embedded_script_entry_point_wrapper_t, __VA_ARGS__)                                          \
                                                                                                                                                               \
        return GLARE_SCRIPT_INLINE_EMBED_INSTANTIATE_SCRIPT_TYPE(embedded_script_t);                                                                           \
    }()

#define GLARE_SCRIPT_LAMBDA_BEGIN()                                                                                        \
    GLARE_SCRIPT_LAMBDA_BEGIN_EX(_embedded_script_entry_point_wrapper_t, util::empty_type<engine::Script>, engine::Script) \
        GLARE_SCRIPT_LAMBDA_STANDARD_DECL()                                                                                \
        {
        
            // {Script lambda body here}

#define GLARE_SCRIPT_LAMBDA_END(...)                                                                                       \
        }                                                                                                                  \
                                                                                                                           \
    GLARE_SCRIPT_LAMBDA_END_EX(_embedded_script_t, _embedded_script_entry_point_wrapper_t, __VA_ARGS__)

#define GLARE_SCRIPT_INLINE_EMBED_EX(embedded_script_t, BaseScriptType, SCRIPT_BODY, ...)               \
    GLARE_SCRIPT_LAMBDA_BEGIN_EX(embedded_script_t##_entry_point_wrapper, BaseScriptType)               \
        SCRIPT_BODY;                                                                                    \
    GLARE_SCRIPT_LAMBDA_END_EX(embedded_script_t, embedded_script_t##_entry_point_wrapper, __VA_ARGS__)

#define GLARE_SCRIPT_LAMBDA_CUSTOM(...) \
    GLARE_SCRIPT_INLINE_EMBED_EX        \
    (                                   \
        embedded_script_t,              \
        engine::Script,                 \
        __VA_ARGS__                     \
    )

#define GLARE_SCRIPT_LAMBDA(...)   \
    GLARE_SCRIPT_INLINE_EMBED_EX   \
    (                              \
        embedded_script_t,         \
        engine::Script,            \
                                   \
        GLARE_SCRIPT_LAMBDA_STANDARD_DECL() \
        __VA_ARGS__                \
    )

#define GLARE_SCRIPT_LAMBDA_DEFINE_FORWARDING_FUNCTIONS(self_var)                                               \
    auto get_registry = [&self_var]() -> decltype(self_var.get_registry()) { return self_var.get_registry(); }; \
    auto get_entity   = [&self_var]() -> decltype(self_var.get_entity())   { return self_var.get_entity();   }; \
    auto get_context  = [&self_var]() -> decltype(self_var.get_context())  { return self_var.get_context();  };

#define GLARE_SCRIPT_IMPL_MAKE_SCRIPT_FN_NO_HEADER_BEGIN_EX(script_name, BaseScriptType)                   \
                {                                                                                          \
                    auto get_registry = [&]() -> decltype(registry_in) { return registry_in; };            \
                    auto get_entity   = [&]() -> decltype(entity_in)   { return entity_in;   };            \
                    auto get_context  = [&]() -> decltype(context_in)  { return context_in;  };            \
                                                                                                           \
                    return                                                                                 \
                    GLARE_SCRIPT_LAMBDA_BEGIN_EX(_embedded_script_entry_point_wrapper_t, BaseScriptType)   \
                        GLARE_SCRIPT_LAMBDA_STANDARD_DECL()                                                \
                        {
        
                            // {Script body here}
                
#define GLARE_SCRIPT_IMPL_MAKE_SCRIPT_FN_NO_HEADER_END_EX()                                                \
                        }                                                                                  \
                                                                                                           \
                    GLARE_SCRIPT_LAMBDA_END_EX(_embedded_script_t, _embedded_script_entry_point_wrapper_t) \
                    ;                                                                                      \
                }

#define GLARE_SCRIPT_IMPL_MAKE_SCRIPT_FN_BEGIN_EX(script_name, BaseScriptType)                \
    auto make_script_##script_name(auto& registry_in, auto entity_in, const auto& context_in) \
        GLARE_SCRIPT_IMPL_MAKE_SCRIPT_FN_NO_HEADER_BEGIN_EX(script_name, BaseScriptType)

            // {Script body here}

#define GLARE_SCRIPT_IMPL_MAKE_SCRIPT_FN_END_EX(...)                                          \
    GLARE_SCRIPT_IMPL_MAKE_SCRIPT_FN_NO_HEADER_END_EX(__VA_ARGS__)

#define GLARE_SCRIPT_IMPL_MAKE_SCRIPT_FN_BEGIN(script_name)                            \
    namespace glare                                                                    \
    {                                                                                  \
        namespace impl                                                                 \
        {                                                                              \
            namespace embedded_script                                                  \
            {                                                                          \
                GLARE_SCRIPT_IMPL_MAKE_SCRIPT_FN_BEGIN_EX(script_name, engine::Script)

                    // {Script body here}

#define GLARE_SCRIPT_IMPL_MAKE_SCRIPT_FN_END(...)                                      \
                GLARE_SCRIPT_IMPL_MAKE_SCRIPT_FN_END_EX(__VA_ARGS__)                   \
            }                                                                          \
        }                                                                              \
    }


#define GLARE_SCRIPT_BEGIN_EX(script_name, BaseScriptType)                                                                                  \
        using script_name =                                                                                                                 \
        decltype                                                                                                                            \
        (                                                                                                                                   \
            [](auto& registry_in, auto entity_in, const auto& context_in)                                                                   \
            GLARE_SCRIPT_IMPL_MAKE_SCRIPT_FN_NO_HEADER_BEGIN_EX(script_name, BaseScriptType)
                
                // {Script body here}
                
#define GLARE_SCRIPT_END_EX(...)                                                                                                            \
            GLARE_SCRIPT_IMPL_MAKE_SCRIPT_FN_NO_HEADER_END_EX(__VA_ARGS__)                                                                  \
            (std::declval<engine::Registry&>(), std::declval<engine::Entity>(), std::declval<const engine::MetaEvaluationContext&>())       \
        );                                                                                                                                  \

#define GLARE_SCRIPT_BEGIN(script_name) \
    GLARE_SCRIPT_BEGIN_EX(script_name, engine::Script)

#define GLARE_SCRIPT_END() \
    GLARE_SCRIPT_END_EX()

#define GLARE_SCRIPT(script_name) \
    GLARE_SCRIPT_BEGIN(script_name)

namespace engine
{
    namespace impl
    {
        class EmbeddedScriptMixin
        {
            public:
                template
			    <
				    typename... EntryPointArgs,

				    typename std::enable_if
				    <
					    (
						    (sizeof...(EntryPointArgs) > 0)
						    &&
						    (
							    !std::is_same_v
							    <
								    std::tuple<EntryPointArgs...>,
								    std::tuple<GLARE_SCRIPT_ENTRYPOINT_PARAMETER_LIST_TYPES>
							    >
						    )
					    ),
                        
					    int
				    >::type=0
			    >
			    ScriptFiber operator()(this auto&& self, EntryPointArgs&&... entry_point_args)
			    {
                    using SelfType = std::decay_t<decltype(self)>;

                    return util::drop_last_until_success<0>
				    (
					    [&](auto&&... args) -> ScriptFiber
					    {
						    if constexpr (std::is_invocable_v<decltype(&SelfType::_script_body), SelfType&, decltype(args)...>)
						    {
							    return self._script_body(std::forward<decltype(args)>(args)...);
						    }
						    else
						    {
							    return ScriptFiber {};
						    }
					    },
				    
					    std::forward<EntryPointArgs>(entry_point_args)...
				    );
			    }
        };
    }
}

//static_assert(GLARE_SCRIPT_IMPL_VA_ARGS_EMPTY());
//static_assert(!GLARE_SCRIPT_IMPL_VA_ARGS_EMPTY(auto&&));
//static_assert(GLARE_SCRIPT_IMPL_EMBEDDED_SCRIPT_IS_LAMBDA([] {}));
//static_assert(!GLARE_SCRIPT_IMPL_EMBEDDED_SCRIPT_IS_LAMBDA({}));

//static_assert(sizeof(decltype(GLARE_SCRIPT_LAMBDA_CUSTOM([]{}))) > 0);
//static_assert(sizeof(decltype(GLARE_SCRIPT_LAMBDA_CUSTOM({}))) > 0);

namespace glare_script_common {}

namespace engine
{
    namespace script {}
}