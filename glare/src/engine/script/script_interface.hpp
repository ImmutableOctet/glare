#pragma once

#include "script_fiber.hpp"
#include "script_entry_point.hpp"

#include <engine/types.hpp>

#include <engine/meta/types.hpp>

#include <optional>

namespace engine
{
	class Service;
	class World;

	struct MetaEvaluationContext;
	struct EntityThread;

	class Script;

	class ScriptInterface
	{
		public:
			// Retrieves a reference to `script` as a `ScriptInterface` object.
			// 
			// This is useful for scenarios where the definition of `Script` is not available,
			// but its relationship as a derived class is important.
			static ScriptInterface& get(Script& script);

			// Retrieves a reference to `script` as a `ScriptInterface` object.
			// See non-const overload for details.
			static const ScriptInterface& get(const Script& script);

			virtual ~ScriptInterface() = 0;

			virtual const MetaEvaluationContext& get_context() const = 0;
			virtual Registry& get_registry() const = 0;
			virtual Entity get_entity() const = 0;

			virtual const Service& get_service() const = 0;
			virtual Service& get_service() = 0;

			virtual World& get_world() = 0;
			virtual const World& get_world() const = 0;

			virtual Entity get_player(PlayerIndex player_index) const = 0;

			virtual EntityThread* get_executing_thread() = 0;
			virtual const EntityThread* get_executing_thread() const = 0;

			virtual std::optional<EntityStateIndex> get_executing_state_index() const { return std::nullopt; }

			virtual EntityThreadID get_executing_thread_name() const { return {}; }
			
			virtual bool waiting_for_event(const MetaType& event_type) const { return false; }
			virtual bool waiting_for_event(MetaTypeID event_type_id)   const { return false; }
			virtual bool waiting_for_event()                           const { return false; }

			virtual MetaAny get_captured_event() const { return {}; }
			virtual MetaAny get_captured_event() { return {}; }

			virtual float get_delta() const { return 1.0f; }

			virtual engine::ScriptFiber operator()() = 0;
			virtual engine::ScriptFiber operator()(GLARE_SCRIPT_ENTRYPOINT_PARAMETER_LIST) = 0;

			virtual explicit operator bool() const { return true; }
	};
}