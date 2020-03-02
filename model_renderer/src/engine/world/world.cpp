#include "world.hpp"
#include "camera.hpp"
#include "debug/debug_camera.hpp"

#include <engine/free_look.hpp>
#include <engine/relationship.hpp>
#include <engine/transform.hpp>

#include <app/input/input.hpp>

namespace engine
{
	World::World()
	{
		register_event<app::input::MouseState, &World::on_mouse_input>();

		auto pivot = create_pivot(*this);
		auto pivot_transform = get_transform(pivot);

		//pivot_transform.set_position({0.0f, 2.0f, 0.0f});

		auto debug_camera = debug::create_debug_camera(*this, CameraParameters::DEFAULT_FOV, pivot);

		cameras.push_back(debug_camera);
	}
	
	/*
	void update_world_matrices(Registry& registry, LocalTransform& local_transform, TransformComponent& world_transform, Entity parent=null)
	{
		if (parent != null)
		{
			auto& parent_relationship    = registry.get<Relationship>(parent);
			auto& parent_local_transform = registry.get<LocalTransform>(parent);
			auto& parent_world_transform = registry.get<TransformComponent>(parent);

			update_world_matrices(registry, parent_local_transform, parent_world_transform, parent_relationship.get_parent());
		}
	}
	*/

	void World::update(float delta)
	{
		event_handler.update();
	}

	Transform World::get_transform(Entity entity)
	{
		return Transform(registry, entity);
	}

	void World::on_mouse_input(const app::input::MouseState& mouse)
	{
		FreeLook::update(*this, mouse);
	}
}