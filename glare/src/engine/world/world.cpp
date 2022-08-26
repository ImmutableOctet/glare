#include "world.hpp"
#include "entity.hpp"
#include "camera.hpp"
#include "physics.hpp"
#include "player.hpp"
#include "stage.hpp"

#include "animator.hpp"
#include "spin_component.hpp"
#include "target_component.hpp"
#include "follow_component.hpp"
#include "billboard_behavior.hpp"
#include "rave_component.hpp"

#include "debug/debug_camera.hpp"
#include "debug/debug_move.hpp"

#include <math/math.hpp>

#include <util/json.hpp>
#include <util/string.hpp>
#include <util/io.hpp>
#include <util/log.hpp>
#include <util/variant.hpp>

#include <graphics/types.hpp>
#include <graphics/canvas.hpp>
#include <graphics/shader.hpp>

#include <engine/config.hpp>
#include <engine/collision.hpp>
#include <engine/resource_manager/resource_manager.hpp>
#include <engine/free_look.hpp>
#include <engine/relationship.hpp>
#include <engine/transform.hpp>
#include <engine/type_component.hpp>
#include <engine/bone_component.hpp>

//#include <engine/name_component.hpp>
#include <engine/components.hpp>
#include <app/input/input.hpp>

#include <algorithm>
#include <fstream>
#include <utility>
//#include <filesystem>

#include <bullet/btBulletCollisionCommon.h>

// Debugging related:
#include <iostream>

namespace engine
{
	World::World(Config& config, ResourceManager& resource_manager, UpdateRate update_rate)
		: config(config), resource_manager(resource_manager), delta_time(update_rate)
	{
		register_event<app::input::MouseState,    &World::on_mouse_input>();
		register_event<app::input::KeyboardState, &World::on_keyboard_input>();
		register_event<OnComponentAdd<engine::CollisionComponent>, &World::on_new_collider>();
		register_event<OnTransformChange, &World::on_transform_change>();
		register_event<OnEntityDestroyed, &World::on_entity_destroyed>();

		subscribe(resource_manager);
		subscribe(physics);
		subscribe(animation);

		root = create_pivot(*this);
		set_name(root, "Root");
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

	World::World(Config& config, ResourceManager& resource_manager, UpdateRate update_rate, const filesystem::path& path)
		: World(config, resource_manager, update_rate)
	{
		load(path);
	}

	World::~World()
	{
		// Not actually needed:
		/*
		unsubscribe(resource_manager);
		unsubscribe(physics);
		unsubscribe(animation);
		*/
	}

	Entity World::load(const filesystem::path& root_path, bool override_current, const std::string& json_file)
	{
		bool is_child_stage = (!override_current) && (stage != null);
		auto parent = root;

		if (is_child_stage)
		{
			parent = stage;
		}

		auto map_data_path = (root_path / json_file).string();
		
		print("Loading map from \"{}\"...", map_data_path);

		std::ifstream map_data_stream(map_data_path);

		//try
		{
			print("Parsing JSON...");

			util::json map_data = util::json::parse(map_data_stream);

			print("Loading...");

			auto map = Stage::Load(*this, parent, root_path, map_data); // { .geometry = false }

			if (!is_child_stage)
			{
				stage = map;
			}

			event<OnStageLoaded>(map, root_path);

			return map;
		}
		/*
		catch (std::exception& e)
		{
			print_warn("Error parsing JSON file: {}", e.what());
			assert(false);
		}
		*/

		return null;
	}

	void World::update(app::Milliseconds time)
	{
		// Update the delta-timer.
		delta_time << time;

		// Update systems:
		Service::update();

		physics.update(*this, delta_time);
		animation.update(*this, delta_time);

		SpinBehavior::update(*this);
		TargetComponent::update(*this);
		SimpleFollowComponent::update(*this);
		BillboardBehavior::update(*this);
		RaveComponent::update(*this);

		handle_transform_events(delta_time);
	}

	void World::handle_transform_events(float delta)
	{
		// Another pass is required, in order to ensure all of the hierarchy has a chance to validate collision representation:
		registry.view<TransformComponent>().each([this](auto entity, auto& tf)
		{
			tf.on_flag(TransformComponent::Flag::EventFlag, [this, entity]()
			{
				this->queue_event<OnTransformChange>(entity);
			});

			//tf.validate(TransformComponent::Dirty::EventFlag);
		});

		/*
		registry.view<TransformComponent, Relationship>().each([&](auto entity, auto& tf, auto& rel)
		{
			auto transform = Transform(registry, entity, rel, tf);

			//transform.validate_collision();
			transform.validate_collision_shallow();
		});
		*/
	}

	void World::update_camera_parameters(int width, int height)
	{
		registry.view<CameraParameters>().each([&](auto entity, auto& camera_component)
		{
			// TODO: Need to look into designating viewports to cameras at the component level.
			// This would make this routine unnecessary for anything but the active camera/main viewport.
			// For now, we just check for the `dynamic_aspect_ratio` flag.
			if (camera_component.dynamic_aspect_ratio)
			{
				camera_component.update_aspect_ratio(width, height);
			}
		});
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

	void World::set_parent
	(
		Entity entity, Entity parent,
		bool defer_action,
		
		bool _null_as_root, bool _is_deferred
	)
	{
		assert(entity != parent);

		/*
		if (_null_as_root)
		{
			if ((parent == null) && (entity != root))
			{
				parent = root;
			}
		}
		*/

		if (defer_action) // && (!_is_deferred)
		{
			print("Deferring parental assignment for: {} to new parent {}", label(entity), label(parent));

			later(&World::set_parent, entity, parent, false, _null_as_root, true);
		}
		else
		{
			//print("Adding child: {} to {}", label(entity), label(parent));

			auto prev_parent = Relationship::set_parent(registry, entity, parent);
			event<OnParentChanged>(entity, prev_parent, parent);
		}
	}

	std::string World::label(Entity entity)
	{
		if (entity == null)
		{
			return "null";
		}

		auto* name_comp = registry.try_get<NameComponent>(entity);

		auto entity_raw = static_cast<EntityIDType>(entity);

		if (name_comp)
		{
			return std::format("\"{}\" ({})", name_comp->name, entity_raw);
		}

		return std::to_string(entity_raw);
	}

	std::string World::get_name(Entity entity)
	{
		auto* name_comp = registry.try_get<NameComponent>(entity);

		if (name_comp)
		{
			return name_comp->name;
		}

		return {};
	}

	void World::set_name(Entity entity, const std::string& name)
	{
		if (!name.empty())
		{
			registry.emplace_or_replace<NameComponent>(entity, name);
		}
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

	Entity World::get_bone_by_name(Entity entity, std::string_view name, bool recursive)
	{
		if (name.empty())
			return null;

		auto* relationship = registry.try_get<Relationship>(entity);

		if (!relationship)
			return null;

		Entity out = null;

		relationship->enumerate_children(registry, [&](Entity child, Relationship& relationship, Entity next_child)
		{
			auto* bone = registry.try_get<BoneComponent>(child);

			if (!bone)
				return true;

			if (bone->name == name)
			{
				out = child;

				return false;
			}
			else if (recursive)
			{
				auto r_out = get_bone_by_name(child, name, true);

				if (r_out != null)
				{
					out = r_out;

					return false;
				}
			}

			return true;
		});

		return out;
	}

	Entity World::get_child_by_name(Entity entity, std::string_view child_name, bool recursive)
	{
		if (child_name.empty())
			return null;

		auto* relationship = registry.try_get<Relationship>(entity);

		if (!relationship)
			return null;

		Entity out = null;

		relationship->enumerate_children(registry, [&](Entity child, Relationship& relationship, Entity next_child)
		{
			auto* name_comp = registry.try_get<NameComponent>(child);

			if (name_comp)
			{
				//print("Comparing {} with {}", name_comp->name, child_name);

				if (name_comp->name == child_name)
				{
					out = child;

					return false;
				}
			}
			
			if (recursive)
			{
				auto r_out = get_child_by_name(child, child_name, true);

				if (r_out != null)
				{
					out = r_out;

					return false;
				}
			}

			return true;
		});

		return out;
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
		assert(registry.try_get<CameraParameters>(camera));

		if ((this->camera == null) || make_active)
		{
			this->camera = camera;
		}

		cameras.push_back(camera);
	}

	void World::remove_camera(Entity camera)
	{
		assert(registry.try_get<CameraParameters>(camera));

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

	void World::on_transform_change(const OnTransformChange& tform_change)
	{
		auto entity = tform_change.entity;

		// Update shadow-maps for light entities.
		update_shadows(*this, entity); // *point_shadows

		// Debugging related:
		/*
		auto* type_component = registry.try_get<TypeComponent>(entity);
		auto entity_type = ((type_component) ? type_component->type : EntityType::Default);

		std::string name_label;

		auto* name_component = registry.try_get<NameComponent>(entity);

		if (name_component)
		{
			name_label = " \"" + name_component->name +"\"";
		}

		print("Entity #{}{} ({}) - Transform Changed: {}", entity, name_label, entity_type, get_transform(entity).get_vectors());
		*/
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