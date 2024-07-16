#pragma once

#include "api.hpp"

#include <engine/types.hpp>

#include <engine/script/script_fiber.hpp>
#include <engine/script/script_handle.hpp>

#include <util/log.hpp>

namespace engine
{
	class Script;

	struct MetaEvaluationContext;
}

namespace glare
{
	using ScriptFiber            = engine::ScriptFiber;
	using ScriptFiberResponse    = engine::ScriptFiberResponse;
	using ScriptControlFlowToken = engine::ScriptControlFlowToken;
	using ScriptHandle           = engine::ScriptHandle;

	using NativeScriptID = engine::StringHash;

	template <NativeScriptID native_script_id>
	engine::ScriptFiber script(engine::Script& self, engine::Registry& registry, engine::Entity entity, const engine::MetaEvaluationContext& context)
	{
		co_yield ScriptControlFlowToken::Complete;
	}

	template <NativeScriptID native_script_id>
	engine::ScriptFiber script(engine::Registry& registry, engine::Entity entity, const engine::MetaEvaluationContext& context)
	{
		co_yield ScriptControlFlowToken::Complete;
	}

	template <NativeScriptID native_script_id>
	engine::ScriptFiber script(engine::ScriptHandle& script_handle_out, engine::Registry& registry, engine::Entity entity, const engine::MetaEvaluationContext& context)
	{
		co_yield ScriptControlFlowToken::Complete;
	}
}