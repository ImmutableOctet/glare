#pragma once

#include "script.hpp"
#include "script_handle.hpp"

#include <engine/types.hpp>

#include <cassert>

namespace engine
{
    struct MetaEvaluationContext;

    namespace impl
    {
        template <typename SelfType, typename MakeScriptFn, typename... ScriptEntryPointArgs>
        ScriptFiber script_bootstrap_in_place_fn_impl
        (
            SelfType& self,

            MakeScriptFn&& make_script_fn,
            
            Registry& registry,
            Entity entity,
            const MetaEvaluationContext context,

            ScriptEntryPointArgs&&... script_args
        )
        {
            using ScriptType = decltype(make_script_fn(registry, entity, context));

            // Ensure that the derived `script_t` is just for exposing the API, and not adding state.
            //static_assert(sizeof(self) == sizeof(ScriptType));

            if (!static_cast<bool>(self))
            {
                // TODO: Look into using an 'any' type here, so we could do something like: self.script = ...;
                self = make_script_fn(registry, entity, context);

                assert(static_cast<bool>(self));
            }

            // NOTE: Not needed, since `sizeof` static-assert ensures assignment using the base type is equivalent.
            //assert(dynamic_cast<ScriptType*>(&self));

            auto& script_instance = static_cast<ScriptType&>(self);

            // TODO: Look at other options to execute call operator overloads safely.
            auto fiber = script_instance(std::forward<ScriptEntryPointArgs>(script_args)...);
            
            co_yield {};

            while (fiber)
            {
                script_instance.on_update();

                co_yield fiber;
            }
        }

        template <typename MakeScriptFn, typename... ScriptEntryPointArgs>
        ScriptFiber script_bootstrap_fn_impl
        (
            ScriptHandle& script_handle_out,
            
            MakeScriptFn&& make_script_fn,

            Registry& registry,
            Entity entity,
            const MetaEvaluationContext context,

            ScriptEntryPointArgs&&... script_args
        )
        {
            auto script_instance = make_script_fn
            (
                registry,
                entity,
                context
            );
            
            auto fiber = script_instance(std::forward<ScriptEntryPointArgs>(script_args)...);

            script_handle_out = &script_instance;

            co_yield {};
                        
            while (fiber)
            {
                script_instance.on_update();

                co_yield fiber;
            }
        }

        template <typename ScriptType, typename... ScriptEntryPointArgs>
        ScriptFiber script_bootstrap_fn
        (
            Registry& registry, Entity entity,
            const MetaEvaluationContext context,
            ScriptEntryPointArgs&&... script_args
        )
        {
            auto unused_script_handle = ScriptHandle {};

            return script_bootstrap_fn_impl
            (
                unused_script_handle,

                [](Registry& registry, Entity entity, const MetaEvaluationContext context)
                {
                    return ScriptType { registry, entity, context };
                },

                registry, entity, context,
                std::forward<ScriptEntryPointArgs>(script_args)...
            );
        }

        template <typename ScriptType, typename SelfType, typename... ScriptEntryPointArgs>
        ScriptFiber script_bootstrap_in_place_fn
        (
            SelfType& self,

            Registry& registry,
            Entity entity,
            const MetaEvaluationContext context,

            ScriptEntryPointArgs&&... script_args
        )
        {
            return script_bootstrap_in_place_fn_impl
            (
                self,

                [](Registry& registry, Entity entity, const MetaEvaluationContext context)
                {
                    return ScriptType { registry, entity, context };
                },

                registry, entity, context,

                std::forward<ScriptEntryPointArgs>(script_args)...
            );
        }
    }
}