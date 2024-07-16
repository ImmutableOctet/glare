#pragma once

#include "script_static_decl.hpp"
#include "code_generation_decl.hpp"

#include <engine/script/code_generation.hpp>
#include <engine/script/script_handle.hpp>
#include <engine/script/script_function_bootstrap.hpp>

#include <cassert>

#define GLARE_SCRIPT_DEFINE_BOOTSTRAP_BINDINGS_EX(script_file_bootstrap_fn, glare_script_type)                                                                                                                          \
    namespace glare                                                                                                                                                                                                     \
    {                                                                                                                                                                                                                   \
        template <>                                                                                                                                                                                                     \
        engine::ScriptFiber script<glare_script_type::script_id>(engine::Script& self, engine::Registry& registry, engine::Entity entity, const engine::MetaEvaluationContext& context)                                 \
        {                                                                                                                                                                                                               \
            return script_file_bootstrap_fn(self, registry, entity, context);                                                                                                                                           \
        }                                                                                                                                                                                                               \
                                                                                                                                                                                                                        \
        template <>                                                                                                                                                                                                     \
        engine::ScriptFiber script<glare_script_type::script_id>(engine::Registry& registry, engine::Entity entity, const engine::MetaEvaluationContext& context)                                                       \
        {                                                                                                                                                                                                               \
            return script_file_bootstrap_fn(registry, entity, context);                                                                                                                                                 \
        }                                                                                                                                                                                                               \
                                                                                                                                                                                                                        \
        template <>                                                                                                                                                                                                     \
        engine::ScriptFiber script<glare_script_type::script_id>(engine::ScriptHandle& script_handle_out, engine::Registry& registry, engine::Entity entity, const engine::MetaEvaluationContext& context)              \
        {                                                                                                                                                                                                               \
            return script_file_bootstrap_fn(script_handle_out, registry, entity, context);                                                                                                                              \
        }                                                                                                                                                                                                               \
                                                                                                                                                                                                                        \
        template <>                                                                                                                                                                                                     \
        engine::ScriptFiber script<glare_script_type::script_no_base_path_id>(engine::Script& self, engine::Registry& registry, engine::Entity entity, const engine::MetaEvaluationContext& context)                    \
        {                                                                                                                                                                                                               \
            return script_file_bootstrap_fn(self, registry, entity, context);                                                                                                                                           \
        }                                                                                                                                                                                                               \
                                                                                                                                                                                                                        \
        template <>                                                                                                                                                                                                     \
        engine::ScriptFiber script<glare_script_type::script_no_base_path_id>(engine::Registry& registry, engine::Entity entity, const engine::MetaEvaluationContext& context)                                          \
        {                                                                                                                                                                                                               \
            return script_file_bootstrap_fn(registry, entity, context);                                                                                                                                                 \
        }                                                                                                                                                                                                               \
                                                                                                                                                                                                                        \
        template <>                                                                                                                                                                                                     \
        engine::ScriptFiber script<glare_script_type::script_no_base_path_id>(engine::ScriptHandle& script_handle_out, engine::Registry& registry, engine::Entity entity, const engine::MetaEvaluationContext& context) \
        {                                                                                                                                                                                                               \
            return script_file_bootstrap_fn(script_handle_out, registry, entity, context);                                                                                                                              \
        }                                                                                                                                                                                                               \
    }

#define GLARE_SCRIPT_DEFINE_BOOTSTRAP_BINDINGS(glare_script_file_library_name) \
    GLARE_SCRIPT_DEFINE_BOOTSTRAP_BINDINGS_EX(glare::glare_script_file_library_name, glare::impl::glare_script_file_library_name::script_t)

#define GLARE_SCRIPT_DEFINE_BOOTSTRAP_FUNCTIONS_EX(script_file_bootstrap_fn, glare_script_type)                                                                                       \
    namespace glare                                                                                                                                                                   \
    {                                                                                                                                                                                 \
        GLARE_SCRIPT_API                                                                                                                                                              \
        ScriptFiber script_file_bootstrap_fn(engine::Script& self, engine::Registry& registry, engine::Entity entity, const engine::MetaEvaluationContext context)                    \
        {                                                                                                                                                                             \
            return engine::impl::script_bootstrap_in_place_fn<glare_script_type>(self, registry, entity, context);                                                                    \
        }                                                                                                                                                                             \
                                                                                                                                                                                      \
        GLARE_SCRIPT_API                                                                                                                                                              \
        ScriptFiber script_file_bootstrap_fn(engine::ScriptHandle& script_handle_out, engine::Registry& registry, engine::Entity entity, const engine::MetaEvaluationContext context) \
        {                                                                                                                                                                             \
            return engine::impl::script_bootstrap_fn_impl                                                                                                                             \
            (                                                                                                                                                                         \
                script_handle_out,                                                                                                                                                    \
                                                                                                                                                                                      \
                [](Registry& registry, Entity entity, const MetaEvaluationContext context)                                                                                            \
                {                                                                                                                                                                     \
                    return glare_script_type { registry, entity, context };                                                                                                                   \
                },                                                                                                                                                                    \
                                                                                                                                                                                      \
                registry, entity, context                                                                                                                                             \
            );                                                                                                                                                                        \
        }                                                                                                                                                                             \
                                                                                                                                                                                      \
        GLARE_SCRIPT_API                                                                                                                                                              \
        ScriptFiber script_file_bootstrap_fn(engine::Registry& registry, engine::Entity entity, const engine::MetaEvaluationContext context)                                          \
        {                                                                                                                                                                             \
            return engine::impl::script_bootstrap_fn<glare_script_type>(registry, entity, context);                                                                                   \
        }                                                                                                                                                                             \
    }

#define GLARE_SCRIPT_DEFINE_BOOTSTRAP_FUNCTIONS(glare_script_file_library_name) \
    GLARE_SCRIPT_DEFINE_BOOTSTRAP_FUNCTIONS_EX(glare_script_file_library_name, glare::impl::glare_script_file_library_name::script_t)