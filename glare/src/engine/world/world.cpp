#include "world.hpp"

#include "entity.hpp"
#include "camera.hpp"
#include "player.hpp"
#include "stage.hpp"
#include "light.hpp"

//#include "animator.hpp"
#include "graphics_entity.hpp"

#include "animation/bone_component.hpp"
#include "physics/collision_component.hpp"

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
#include <engine/resource_manager/resource_manager.hpp>
#include <engine/relationship.hpp>

#include <engine/type_component.hpp>
#include <engine/name_component.hpp>

#include <app/input/input.hpp>

#include <algorithm>
#include <fstream>
#include <utility>
//#include <filesystem>

//#include <bullet/btBulletCollisionCommon.h>

namespace engine
{
	World::World(Config& config, ResourceManager& resource_manager, UpdateRate update_rate)
		: config(config), resource_manager(resource_manager), delta_time(update_rate)
	{
		register_event<OnTransformChanged, &World::on_transform_change>();
		register_event<OnEntityDestroyed, &World::on_entity_destroyed>();

		subscribe(resource_manager);

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
		// unsubscribe(...);
	}

	Registry& World::get_registry()
	{
		return registry;
	}

	const Registry& World::get_registry() const
	{
		return registry;
	}

	Entity World::load(const filesystem::path& root_path, const std::string& json_file, Entity parent)
	{
		auto map_data_path = (root_path / json_file).string();
		
		print("Loading map from \"{}\"...", map_data_path);

		std::ifstream map_data_stream(map_data_path);

		//try
		{
			print("Parsing JSON...");

			util::json map_data = util::json::parse(map_data_stream);

			print("Loading...");

			auto map = Stage::Load(*this, root_path, map_data, parent); // { .geometry = false }

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

		// Handle changes in entity transforms.
		handle_transform_events(delta_time);

		// Update systems:
		Service::update(delta_time);
	}

	void World::handle_transform_events(float delta)
	{
		// Another pass is required, in order to ensure all of the hierarchy has a chance to validate collision representation:
		registry.view<TransformComponent>().each([this](auto entity, auto& tf)
		{
			tf.on_flag(TransformComponent::Flag::EventFlag, [this, entity]()
			{
				//this->queue_event<OnTransformChanged>(entity);
				this->event<OnTransformChanged>(entity);
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

	void World::apply_transform_and_reset_collision(Entity entity, const math::TransformVectors& tform_data)
	{
		transform_and_reset_collision(entity, [&tform_data](Transform& tform)
		{
			tform.apply(tform_data);
		});
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

	math::Vector World::get_gravity() const
	{
		return properties.gravity;
	}

	void World::set_gravity(const math::Vector& gravity)
	{
		// Temporarily store the old gravity vector.
		auto old_gravity = properties.gravity;
		
		// Assign the newly provided gravity vector.
		properties.gravity = gravity;

		// Notify listeners that the world's gravity has changed.
		event<OnGravityChanged>(old_gravity, gravity);
	}

	void World::set_properties(const WorldProperties& properties)
	{
		constexpr bool trigger_events = true;

		// Event-enabled properties:
		if constexpr (trigger_events)
		{
			set_gravity(properties.gravity);
		}

		this->properties = properties;
	}

	math::Vector World::down() const
	{
		//return { 0.0f, -1.0f, 0.0f };
		return glm::normalize(get_gravity());
	}

	void World::set_camera(Entity camera)
	{
		assert(registry.try_get<CameraParameters>(camera));

		this->camera = camera;
	}

	void World::on_transform_change(const OnTransformChanged& tform_change)
	{
		auto entity = tform_change.entity;

		// TODO: Move this into a separate lighting system/event-handler...?
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

	// This exists purely to circumvent having a complete type definition for `CollisionComponent` in the header.
	CollisionComponent* World::get_collision_component(Entity entity)
	{
		return registry.try_get<CollisionComponent>(entity);
	}

	bool World::_transform_and_reset_collision_store_active_state(CollisionComponent* collision)
	{
		bool collision_active = false;

		// Null-check added for safety.
		if (collision)
		{
			collision_active = collision->is_active();

			collision->deactivate();
		}

		return collision_active;
	}

	void World::_transform_and_reset_collision_revert_active_state(CollisionComponent* collision, const math::Matrix& tform_matrix, bool collision_active)
	{
		// Null-check added for safety.
		if (collision)
		{
			/*
				This will trigger a `OnTransformChanged` event, which
				in turn *may* update the collision-object again.

				Regardless, we still perform a transform update/assignment to
				ensure that nothing is out-of-sync with the physics engine.
			*/
			collision->update_transform(tform_matrix);

			if (collision_active)
			{
				collision->activate();
			}
		}
	}
}