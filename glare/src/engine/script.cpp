#include "script.hpp"

#include <engine/entity/entity_system.hpp>
#include <engine/entity/entity_state.hpp>

#include <engine/world/world.hpp>
#include <engine/world/delta/delta_system.hpp>

namespace engine
{
	Script::Script
	(
		const MetaEvaluationContext& context,
		Registry& registry,
		ScriptID script_id,
		Entity entity
	) :
		_context(&context),
		_registry(&registry),
		_script_id(script_id),
		_entity(entity)
	{}

	bool Script::on_update()
	{
		update_state_references();
		update_deltatime();

		return true;
	}

	void Script::update_state_references()
	{
		state      = get_state_id();
		prev_state = get_prev_state_id();
	}

	void Script::update_deltatime()
	{
		delta = get_delta();
	}

	const EntityState* Script::get_state_description() const
	{
		return system<EntitySystem>().get_state(*this);
	}

	const EntityState* Script::get_prev_state_description() const
	{
		return system<EntitySystem>().get_prev_state(*this);
	}

	EntityStateID Script::get_state_id() const
	{
		if (const auto state = get_state_description())
		{
			if (state->name)
			{
				return *state->name;
			}
		}

		return {};
	}

	EntityStateID Script::get_prev_state_id() const
	{
		if (const auto prev_state = get_prev_state_description())
		{
			if (prev_state->name)
			{
				return *prev_state->name;
			}
		}

		return {};
	}

	World& Script::get_world()
	{
		auto& service = get_service();

		auto world = dynamic_cast<engine::World*>(&service);

		assert(world);

		return *world;
	}

	const World& Script::get_world() const
	{
		auto& service = get_service();

		auto world = dynamic_cast<const engine::World*>(&service);

		assert(world);

		return *world;
	}

	DeltaSystem& Script::get_delta_system()
	{
		return system<engine::DeltaSystem>();
	}

	const DeltaSystem& Script::get_delta_system() const
	{
		return system<engine::DeltaSystem>();
	}

	float Script::get_delta() const
	{
		return get_delta_system().get_delta();
	}
}