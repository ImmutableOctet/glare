#pragma once

#include "api.hpp"

#include <engine/types.hpp>

#include <engine/entity/script_fiber.hpp>

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

	using NativeScriptID = engine::StringHash;

	template <NativeScriptID native_script_id>
	ScriptFiber script(engine::Script& self, engine::Registry& registry, engine::Entity entity, const engine::MetaEvaluationContext& context)
	{
		co_yield ScriptControlFlowToken::Complete;
	}

	template <NativeScriptID native_script_id>
	ScriptFiber script(engine::Registry& registry, engine::Entity entity, const engine::MetaEvaluationContext& context)
	{
		co_yield ScriptControlFlowToken::Complete;
	}
}