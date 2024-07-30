#include "script.hpp"

#include <engine/entity/entity_system.hpp>
#include <engine/entity/entity_state.hpp>

#include <engine/world/world.hpp>
#include <engine/world/delta/delta_system.hpp>

#include <engine/lod/entity_batch_system.hpp>
#include <engine/lod/components/level_of_detail_component.hpp>

#include <engine/meta/meta_evaluation_context.hpp>

namespace engine
{
	Script::Script
	(
		const MetaEvaluationContext& context,
		Registry& registry,
		ScriptID script_id,
		Entity entity
	) :
		ScriptBase(context, registry, script_id, entity)
	{}

	Script::~Script() {}

	bool Script::on_update()
	{
		update_deltatime();

		return true;
	}

	void Script::update_deltatime()
	{
		delta = get_delta();
	}

	engine::ScriptFiber Script::operator()()
	{
		return this->basic_call_operator_impl();
	}

	MetaAny Script::get_captured_event()
	{
		if (waiting_for_event())
		{
			return {};
		}

		return _captured_event.as_ref();
	}

	MetaAny Script::get_captured_event() const
	{
		if (waiting_for_event())
		{
			return {};
		}

		return _captured_event.as_ref();
	}

	void Script::set_pending_event_type(const MetaType& event_type)
	{
		if (event_type)
		{
			set_pending_event_type(event_type.id());
		}
	}

	void Script::set_pending_event_type(MetaTypeID event_type_id)
	{
		_captured_event = event_type_id;
	}

	void Script::set_captured_event(MetaAny&& newly_captured_event)
	{
		if (!newly_captured_event)
		{
			return;
		}

		if ((!_captured_event) || waiting_for_event(newly_captured_event.type()))
		{
			_captured_event = std::move(newly_captured_event);
		}
	}

	void Script::clear_captured_event()
	{
		if (!waiting_for_event())
		{
			_captured_event = {};
		}
	}

	bool Script::waiting_for_event(const MetaType& event_type) const
	{
		return waiting_for_event(event_type.id());
	}

	bool Script::waiting_for_event(MetaTypeID event_type_id) const
	{
		if (const auto pending_type_id = _captured_event.try_cast<MetaTypeID>())
		{
			return ((*pending_type_id == MetaTypeID {}) || (*pending_type_id == event_type_id));
		}

		return false;
	}

	bool Script::waiting_for_event() const
	{
		return static_cast<bool>(_captured_event.try_cast<MetaTypeID>());
	}

	World& Script::get_world()
	{
		auto& service = get_service();

		auto world = dynamic_cast<World*>(&service);

		assert(world);

		return *world;
	}

	const World& Script::get_world() const
	{
		auto& service = get_service();

		auto world = dynamic_cast<const World*>(&service);

		assert(world);

		return *world;
	}

	float Script::get_delta() const
	{
		if (const auto level_of_detail_component = try_get<LevelOfDetailComponent>())
		{
			if (const auto batch_system = try_get_system<EntityBatchSystem>())
			{
				return batch_system->get_delta(level_of_detail_component->update_level);
			}
		}

		if (const auto delta_system = try_get_system<DeltaSystem>())
		{
			return delta_system->get_delta();
		}

		return 1.0f;
	}

	EntityThread* Script::get_executing_thread()
	{
		return _executing_thread;
	}

	const EntityThread* Script::get_executing_thread() const
	{
		return _executing_thread;
	}

	std::optional<EntityStateIndex> Script::get_executing_state_index() const
	{
		if (const auto thread = get_executing_thread())
		{
			return thread->state_index;
		}

		return std::nullopt;
	}

	EntityThreadID Script::get_executing_thread_name() const
	{
		if (const auto thread = get_executing_thread())
		{
			return thread->thread_id;
		}
				
		return {};
	}

	MetaTypeID Script::get_pending_event_type_id() const
	{
		if (const auto pending_type_id = _captured_event.try_cast<MetaTypeID>())
		{
			return *pending_type_id;
		}

		return {};
	}

	bool Script::has_yield_continuation_predicate() const
	{
		return static_cast<bool>(_yield_continuation_predicate);
	}

	const Script::YieldContinuationPredicate& Script::get_yield_continuation_predicate() const
	{
		return _yield_continuation_predicate;
	}

	void Script::set_executing_thread(EntityThread& thread)
	{
		_executing_thread = &thread;
	}

	void Script::clear_executing_thread()
	{
		_executing_thread = {};
	}
}