#pragma once

#include "api.hpp"
#include "script_static_decl.hpp"

#include <engine/script/code_generation.hpp>
#include <engine/script/script_handle.hpp>

#define GLARE_SCRIPT_DECLARE_BOOTSTRAP_BINDINGS(script_file_path)                                                                                                                                                       \
    namespace glare                                                                                                                                                                                                     \
    {                                                                                                                                                                                                                   \
        template <>                                                                                                                                                                                                     \
        engine::ScriptFiber script<engine::hash(script_file_path)>(engine::Script& self, engine::Registry& registry, engine::Entity entity, const engine::MetaEvaluationContext& context);                              \
                                                                                                                                                                                                                        \
        template <>                                                                                                                                                                                                     \
        engine::ScriptFiber script<engine::hash(script_file_path)>(engine::Registry& registry, engine::Entity entity, const engine::MetaEvaluationContext& context);                                                    \
                                                                                                                                                                                                                        \
        template <>                                                                                                                                                                                                     \
        engine::ScriptFiber script<engine::hash(script_file_path)>(engine::ScriptHandle& script_handle_out, engine::Registry& registry, engine::Entity entity, const engine::MetaEvaluationContext& context);           \
                                                                                                                                                                                                                        \
        template <>                                                                                                                                                                                                     \
        engine::ScriptFiber script<engine::hash("engine/" script_file_path)>(engine::Script& self, engine::Registry& registry, engine::Entity entity, const engine::MetaEvaluationContext& context);                    \
                                                                                                                                                                                                                        \
        template <>                                                                                                                                                                                                     \
        engine::ScriptFiber script<engine::hash("engine/" script_file_path)>(engine::Registry& registry, engine::Entity entity, const engine::MetaEvaluationContext& context);                                          \
                                                                                                                                                                                                                        \
        template <>                                                                                                                                                                                                     \
        engine::ScriptFiber script<engine::hash("engine/" script_file_path)>(engine::ScriptHandle& script_handle_out, engine::Registry& registry, engine::Entity entity, const engine::MetaEvaluationContext& context); \
    }

#define GLARE_SCRIPT_DECLARE_BOOTSTRAP_FUNCTIONS(script_file_bootstrap_fn)                                                                                                             \
    namespace glare                                                                                                                                                                    \
    {                                                                                                                                                                                  \
        GLARE_SCRIPT_API                                                                                                                                                               \
        ScriptFiber script_file_bootstrap_fn(engine::Script& self, engine::Registry& registry, engine::Entity entity, const engine::MetaEvaluationContext context);                    \
                                                                                                                                                                                       \
        GLARE_SCRIPT_API                                                                                                                                                               \
        ScriptFiber script_file_bootstrap_fn(engine::Registry& registry, engine::Entity entity, const engine::MetaEvaluationContext context);                                          \
                                                                                                                                                                                       \
        GLARE_SCRIPT_API                                                                                                                                                               \
        ScriptFiber script_file_bootstrap_fn(engine::ScriptHandle& script_handle_out, engine::Registry& registry, engine::Entity entity, const engine::MetaEvaluationContext context); \
    }