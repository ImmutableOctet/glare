#include "world.hpp"
#include "camera.hpp"

#include "debug/debug_camera.hpp"
#include "debug/debug_move.hpp"

#include <graphics/canvas.hpp>
#include <graphics/shader.hpp>

#include <engine/free_look.hpp>
#include <engine/relationship.hpp>
#include <engine/transform.hpp>

#include <engine/components/graphics/graphics.hpp>

#include <app/input/input.hpp>

#include <algorithm>

namespace engine
{
	World::World(const app::DeltaTime& delta_time)
		: delta_time(delta_time)
	{
		register_event<app::input::MouseState,    &World::on_mouse_input>();
		register_event<app::input::KeyboardState, &World::on_keyboard_input>();

		root = create_pivot(*this);
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

	void World::update()
	{
		event_handler.update();
	}

	bool World::render(graphics::Canvas& canvas, bool forward_rendering)
	{
		return render(canvas, this->camera, forward_rendering);
	}

	bool World::render(graphics::Canvas& canvas, Entity camera, bool forward_rendering)
	{
		// Deferred referning is current unsupported.
		ASSERT(forward_rendering);

		if (camera == null)
		{
			return false;
		}

		auto& registry = get_registry();

		//auto& camera_transform = registry.get<engine::TransformComponent>(camera);
		//auto inverse_world = camera_transform.inverse_world;

		auto camera_transform = get_transform(camera);
		auto& camera_params = registry.get<engine::CameraParameters>(camera);

		math::Matrix camera_matrix = camera_transform.get_inverse_matrix();

		// TODO: Move this aspect-ratio update to an event triggered on window-resize.
		///camera_params.aspect_ratio = window->horizontal_aspect_ratio();

		auto& shader = canvas.get_shader();

		shader["projection"] = glm::perspective(camera_params.fov, camera_params.aspect_ratio, camera_params.near, camera_params.far);
		shader["view"] = camera_matrix;

		registry.view<ModelComponent, TransformComponent, Relationship>().each([&](auto entity, auto& model_component, auto& transform, const auto& relationship)
		{
			if (!model_component.visible)
			{
				return;
			}

			/*
			if (model_component.transparent())
			{
				return;
			}
			*/

			if (model_component.model == nullptr)
			{
				return;
			}

			auto model_transform = Transform(registry, entity, relationship, transform);
			auto model_matrix = model_transform.get_matrix();

			shader["model"] = model_matrix;

			auto& model = *model_component.model;
			auto color = model_component.color;

			canvas.draw(model, color, graphics::Canvas::DrawMode::All);
		});

		return true;
	}

	Transform World::get_transform(Entity entity)
	{
		return Transform(registry, entity);
	}

	void World::add_camera(Entity camera)
	{
		ASSERT(registry.has<CameraParameters>(camera));

		if (this->camera == null)
		{
			this->camera = camera;
		}

		cameras.push_back(camera);
	}

	void World::remove_camera(Entity camera)
	{
		ASSERT(registry.has<CameraParameters>(camera));

		auto it = std::find(cameras.begin(), cameras.end(), camera);

		if (it != cameras.end())
		{
			cameras.erase(it);
		}

		if (cameras.empty())
		{
			this->camera = null;
		}
		else
		{
			this->camera = cameras[0];
		}
	}

	void World::on_mouse_input(const app::input::MouseState& mouse)
	{
		FreeLook::update(*this, mouse);
	}

	void World::on_keyboard_input(const app::input::KeyboardState& keyboard)
	{
		engine::debug::DebugMove::update(*this, keyboard);
	}
}