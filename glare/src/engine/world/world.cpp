#include "world.hpp"
#include "entity.hpp"
#include "camera.hpp"
#include "physics.hpp"
#include "player.hpp"
#include "stage.hpp"

#include "spin_component.hpp"
#include "target_component.hpp"
#include "follow_component.hpp"
#include "billboard_behavior.hpp"
#include "rave_component.hpp"

#include "debug/debug_camera.hpp"
#include "debug/debug_move.hpp"

#include <math/math.hpp>

#include <util/json.hpp>
#include <util/io.hpp>
#include <util/log.hpp>

#include <graphics/canvas.hpp>
#include <graphics/shader.hpp>

#include <engine/collision.hpp>
#include <engine/resource_manager.hpp>
#include <engine/free_look.hpp>
#include <engine/relationship.hpp>
#include <engine/transform.hpp>

//#include <engine/name_component.hpp>
#include <engine/components.hpp>
#include <app/input/input.hpp>

#include <algorithm>
#include <fstream>
//#include <filesystem>

#include <bullet/btBulletCollisionCommon.h>

// Debugging related:
#include <iostream>

namespace engine
{
	World::World(ResourceManager& resource_manager, UpdateRate update_rate)
		: resource_manager(resource_manager), delta_time(update_rate)
	{
		register_event<app::input::MouseState,    &World::on_mouse_input>();
		register_event<app::input::KeyboardState, &World::on_keyboard_input>();
		register_event<OnComponentAdd<engine::CollisionComponent>, &World::on_new_collider>();
		register_event<OnEntityDestroyed, &World::on_entity_destroyed>();

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

	World::World(ResourceManager& resource_manager, UpdateRate update_rate, const filesystem::path& path)
		: World(resource_manager, update_rate)
	{
		load(path);
	}

	Entity World::load(const filesystem::path& root_path, bool override_current, const std::string& json_file)
	{
		auto con = util::log::get_console();

		bool is_child_stage = (!override_current) && (stage != null);
		auto parent = root;

		if (is_child_stage)
		{
			parent = stage;
		}

		auto map_data_path = (root_path / json_file).string();
		
		con->info("Loading map from \"{}\"...", map_data_path);

		con->info("Parsing JSON...");

		std::ifstream map_data_stream(map_data_path);

		try
		{
			util::json map_data = util::json::parse(map_data_stream);

			auto map = Stage::Load(*this, parent, con, root_path, map_data);

			if (!is_child_stage)
			{
				stage = map;
			}

			return map;
		}
		catch (std::exception& e)
		{
			con->error("Error parsing JSON file: {}", e.what());
			ASSERT(false);
		}

		return null;
	}

	void World::update(app::Milliseconds time)
	{
		// Update the delta-timer.
		delta_time << time;

		event_handler.update();

		// Update systems:
		physics.update(*this, delta_time);

		SpinBehavior::update(*this);
		TargetComponent::update(*this);
		SimpleFollowComponent::update(*this);
		BillboardBehavior::update(*this);
		RaveComponent::update(*this);
	}

	bool World::render(graphics::Canvas& canvas, const graphics::Viewport& viewport, bool multi_pass, bool use_active_shader, graphics::CanvasDrawMode additional_draw_modes)
	{
		return render(canvas, viewport, this->camera, multi_pass, use_active_shader, additional_draw_modes);
	}

	bool World::render(graphics::Canvas& canvas, const graphics::Viewport& viewport, Entity camera, bool multi_pass, bool use_active_shader, graphics::CanvasDrawMode additional_draw_modes)
	{
		// Deferred referning is current unsupported.
		//ASSERT(forward_rendering);

		if (camera == null)
		{
			return false;
		}

		auto& registry = get_registry();

		//auto& camera_transform = registry.get<engine::TransformComponent>(camera);
		//auto inverse_world = camera_transform.inverse_world;

		auto camera_transform = get_transform(camera);
		auto& camera_params = registry.get<engine::CameraParameters>(camera);

		math::Matrix camera_matrix;

		if (camera_params.free_rotation)
		{
			camera_matrix = camera_transform.get_inverse_matrix();
		}
		else
		{
			camera_matrix = camera_transform.get_camera_matrix();
		}

		// TODO: Move this aspect-ratio update to an event triggered on window-resize.
		///camera_params.aspect_ratio = window->horizontal_aspect_ratio();

		math::Matrix projection;

		switch (camera_params.projection_mode)
		{
			case CameraProjection::Orthographic:
			{
				float width = static_cast<float>(viewport.get_width());
				float height = static_cast<float>(viewport.get_height());

				float hw = (width / 2.0f);
				float hh = (height / 2.0f);

				projection = glm::ortho(-hw, hw, hh, -hh, camera_params.near_plane, camera_params.far_plane);

				break;
			}
			default: // case CameraProjection::Perspective:
				projection = glm::perspective(camera_params.fov, camera_params.aspect_ratio, camera_params.near_plane, camera_params.far_plane);

				break;
		}

		if (multi_pass)
		{
			draw_models((graphics::CanvasDrawMode::Opaque | additional_draw_modes), canvas, projection, camera_matrix, use_active_shader);
			draw_models((graphics::CanvasDrawMode::Transparent | additional_draw_modes), canvas, projection, camera_matrix, use_active_shader);
		}
		else
		{
			draw_models((graphics::Canvas::DrawMode::All | additional_draw_modes), canvas, projection, camera_matrix, use_active_shader);
		}

		return true;
	}

	void World::draw_models(graphics::CanvasDrawMode draw_mode, graphics::Canvas& canvas, const math::Matrix& projection_matrix, const math::Matrix& view_matrix, bool use_active_shader) // graphics::Shader& shader
	{
		auto draw = [&, this](auto& shader)
		{
			shader["projection"] = projection_matrix;
			shader["view"] = view_matrix;

			bool _auto_clear_textures = false; // true;

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
				auto model_matrix = model_transform.get_matrix(true); // get_local_matrix();
				//auto model_matrix = model_transform.get_local_matrix();

				shader["model"] = model_matrix;

				auto& model = *model_component.model;
				auto color  = model_component.color;

				canvas.draw(model, color, draw_mode, _auto_clear_textures);
			});
		};

		if ((use_active_shader) || (draw_mode & graphics::CanvasDrawMode::IgnoreShaders))
		{
			auto& shader = canvas.get_shader();

			draw(shader);
		}
		else
		{
			auto& shaders = resource_manager.loaded_shaders;

			for (auto& shader_entry : shaders)
			{
				auto& shader_ptr = shader_entry.second;

				if (!shader_ptr)
				{
					continue;
				}

				auto& ctx = canvas.get_context();
				auto& shader = *shader_ptr;

				ctx.use(shader, [&, this]()
				{
					draw(shader);
				});
			}
		}
	}

	Transform World::apply_transform(Entity entity, const math::TransformVectors& tform)
	{
		auto transform = get_transform(entity);

		transform.apply(tform);

		return transform;
	}

	Transform World::set_position(Entity entity, const math::Vector& position)
	{
		auto transform = get_transform(entity);

		transform.set_position(position);

		return transform;
	}

	Transform World::get_transform(Entity entity)
	{
		return Transform(registry, entity);
	}

	math::Vector World::get_up_vector(math::Vector up) const
	{
		auto& m = registry.get<TransformComponent>(root);

		return (m._w * math::Vector4D{ up, 1.0f });
	}

	Entity World::get_parent(Entity entity) const
	{
		auto* relationship = registry.try_get<Relationship>(entity);

		if (relationship == nullptr)
		{
			return null;
		}

		return relationship->get_parent();
	}

	void World::set_parent(Entity entity, Entity parent, bool _null_as_root)
	{
		/*
		if (_null_as_root)
		{
			if ((parent == null) && (entity != root))
			{
				parent = root;
			}
		}
		*/

		Relationship::set_parent(registry, entity, parent);
	}

	Entity World::get_by_name(std::string_view name)
	{
		auto view = registry.view<NameComponent>();

		for (auto it = view.begin(); it != view.end(); it++)
		{
			Entity entity = *it;

			auto name_comp = registry.get<NameComponent>(entity);

			if (name_comp.name == name)
			{
				return entity;
			}
		}

		return null;
	}

	Entity World::get_player(PlayerIndex player) const
	{
		auto view = registry.view<PlayerState>();

		for (auto it = view.begin(); it != view.end(); it++)
		{
			Entity entity = *it;

			const auto& player_state = registry.get<PlayerState>(entity);

			if (player_state.index == player)
			{
				return entity;
			}
		}

		return null;
	}

	void World::add_camera(Entity camera, bool make_active)
	{
		ASSERT(registry.has<CameraParameters>(camera));

		if ((this->camera == null) || make_active)
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

	void World::on_new_collider(const OnComponentAdd<CollisionComponent>& new_col)
	{
		physics.on_new_collider(*this, new_col);
	}

	void World::on_entity_destroyed(const OnEntityDestroyed& destruct)
	{
		auto entity          = destruct.entity;
		auto parent          = destruct.parent;
		auto type            = destruct.type;
		auto destroy_orphans = destruct.destroy_orphans;

		// Handle collision:
		auto* col = registry.try_get<CollisionComponent>(entity);

		if (col)
		{
			physics.on_destroy_collider(*this, entity, *col);
		}

		// Handle entity relationships:
		auto* relationship = registry.try_get<Relationship>(entity);

		if (relationship) // <-- Null-check for 'relationship' isn't actually necessary. (MSVC complains)
		{
			// Make the assumption this object exists, as if it didn't, we would be in an invalid state.
			auto* parent_relationship = registry.try_get<Relationship>(parent);

			relationship->enumerate_child_entities(registry, [&](Entity child, Entity next_child) -> bool
			{
				if (parent_relationship)
				{
					parent_relationship->add_child(registry, parent, child);
				}
				else
				{
					set_parent(child, null);
				}

				if (destroy_orphans)
				{
					// Queue orphans to be destroyed on next event cycle.
					destory_entity(*this, child, true);
				}

				return true;
			});

			//registry.replace<Relationship>(parent, std::move(parent_relationship)); // [&](auto& r) { r = parent_relationship; }
		}

		registry.destroy(entity);
	}
}