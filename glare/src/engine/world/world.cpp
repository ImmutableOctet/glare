#include "world.hpp"

#include "world_events.hpp"
#include "entity.hpp"
#include "scene.hpp"

#include "light.hpp"
#include "light_type.hpp"

//#include "animator.hpp"
#include "graphics_entity.hpp"

#include "components/light_component.hpp"
#include "components/directional_light_component.hpp"
#include "components/directional_light_shadow_component.hpp"
#include "components/point_light_component.hpp"
#include "components/point_light_shadow_component.hpp"
#include "components/spot_light_component.hpp"
//#include "components/spot_light_shadow_component.hpp"

#include "animation/components/bone_component.hpp"
#include "animation/components/skeletal_component.hpp"

#include "physics/components/collision_component.hpp"

#include "camera/camera_system.hpp"

#include "delta/delta_system.hpp"

#include <engine/components/model_component.hpp>

#include <engine/entity/entity_descriptor.hpp>
#include <engine/entity/components/instance_component.hpp>

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

#include <engine/meta/hash.hpp>

#include <engine/resource_manager/resource_manager.hpp>
#include <engine/resource_manager/animation_data.hpp>

#include <engine/components/relationship_component.hpp>
#include <engine/components/forwarding_component.hpp>

#include <engine/components/type_component.hpp>
#include <engine/components/transform_history_component.hpp>
//#include <engine/components/name_component.hpp>

#include <algorithm>
#include <fstream>
#include <utility>
//#include <filesystem>

//#include <bullet/btBulletCollisionCommon.h>

namespace engine
{
	void* World::ensure_light_sub_component(Registry& registry, Entity entity, LightType type)
	{
		switch (type)
		{
			case LightType::Point:
				return &(registry.get_or_emplace<PointLightComponent>(entity));
			case LightType::Directional:
				return &(registry.get_or_emplace<DirectionalLightComponent>(entity));
			case LightType::Spot:
				return &(registry.get_or_emplace<SpotLightComponent>(entity));
			default:
				return {};
		}
	}

	std::size_t World::remove_light_sub_components(Registry& registry, Entity entity, std::optional<LightType> except_type)
	{
		//assert(registry.try_get<LightComponent>(entity));

		std::size_t components_removed = 0;

		if (except_type && (*except_type == LightType::Any))
		{
			return components_removed; // 0;
		}

		if (!except_type || (*except_type != LightType::Point))
		{
			components_removed += registry.remove<PointLightComponent>(entity);
		}

		if (!except_type || (*except_type != LightType::Directional))
		{
			components_removed += registry.remove<DirectionalLightComponent>(entity);
		}

		if (!except_type || (*except_type != LightType::Spot))
		{
			components_removed += registry.remove<SpotLightComponent>(entity);
		}

		return components_removed;
	}

	World::World
	(
		Registry& registry,
		SystemManagerInterface& systems,
		Config& config,
		ResourceManager& resource_manager,
		UpdateRate update_rate
	) :
		Service(registry, systems),
		config(config),
		resource_manager(resource_manager)
	{
		registry.emplace<TransformComponent>(root);

		set_name(root, "Root");

		register_event<OnTransformChanged, &World::on_transform_changed>(*this);

		registry.on_construct<InstanceComponent>().connect<&World::on_instance>(*this);

		registry.on_construct<LightComponent>().connect<&World::on_light_init>(*this);
		registry.on_update<LightComponent>().connect<&World::on_light_update>(*this);
		registry.on_destroy<LightComponent>().connect<&World::on_light_destroyed>(*this);

		registry.on_construct<DirectionalLightComponent>().connect<&World::on_directional_light_init>(*this);
		registry.on_update<DirectionalLightComponent>().connect<&World::on_directional_light_update>(*this);
		registry.on_destroy<DirectionalLightComponent>().connect<&World::on_directional_light_destroyed>(*this);
		
		registry.on_construct<PointLightComponent>().connect<&World::on_point_light_init>(*this);
		registry.on_update<PointLightComponent>().connect<&World::on_point_light_update>(*this);
		registry.on_destroy<PointLightComponent>().connect<&World::on_point_light_destroyed>(*this);

		registry.on_construct<SpotLightComponent>().connect<&World::on_spot_light_init>(*this);
		registry.on_update<SpotLightComponent>().connect<&World::on_spot_light_update>(*this);
		registry.on_destroy<SpotLightComponent>().connect<&World::on_spot_light_destroyed>(*this);

		subscribe(resource_manager);
	}
	
	/*
	void update_world_matrices(Registry& registry, LocalTransform& local_transform, TransformComponent& world_transform, Entity parent=null)
	{
		if (parent != null)
		{
			auto& parent_relationship    = registry.get<RelationshipComponent>(parent);
			auto& parent_local_transform = registry.get<LocalTransform>(parent);
			auto& parent_world_transform = registry.get<TransformComponent>(parent);

			update_world_matrices(registry, parent_local_transform, parent_world_transform, parent_relationship.get_parent());
		}
	}
	*/

	World::World
	(
		Registry& registry,
		SystemManagerInterface& systems,
		Config& config,
		ResourceManager& resource_manager,
		UpdateRate update_rate,
		const filesystem::path& path
	) :
		World(registry, systems, config, resource_manager, update_rate)
	{
		load(path);
	}

	World::~World()
	{
		// unsubscribe(...);
	}

	Entity World::load
	(
		const filesystem::path& scene_root_path,
		std::string_view scene_filename,
		Entity parent,
		const SceneLoaderConfig& cfg,
		SystemManagerInterface* opt_system_manager
	)
	{
		auto scene_data_path = (scene_root_path / scene_filename);
		
		print("Loading scene from \"{}\"...", scene_data_path.string());

		//try
		{
			print("Parsing scene JSON...");

			auto scene_data = util::load_json(scene_data_path);

			print("Loading scene contents...");

			auto scene_loader = SceneLoader(*this, scene_root_path, null, cfg, opt_system_manager);

			auto scene = scene_loader.load(scene_data, parent);

			event<OnSceneLoaded>(scene, scene_root_path); // scene_data_path

			return scene;
		}
		/*
		catch (std::exception& e)
		{
			print_warn("Error parsing scene JSON file: {}", e.what());

			assert(false);
		}
		*/

		return null;
	}

	void World::update(TimePoint time)
	{
		float delta = 1.0f;
		
		if (auto delta_system = get_delta_system())
		{
			delta = delta_system->update_delta(time);
		}

		// Handle changes in entity transforms.
		handle_transform_events(delta);

		// Update systems.
		Service::update(time, delta);
	}

	Entity World::get_forwarded(Entity entity)
	{
		auto* forwarding = registry.try_get<ForwardingComponent>(entity);

		if (forwarding)
		{
			return forwarding->root_entity;
		}

		return entity;
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

	const AnimationData* World::get_animation_data(Entity entity) const
	{
		return get_animation_data(get_registry(), entity);
	}

	const AnimationData* World::get_animation_data(Registry& registry, Entity entity) const
	{
		if (const auto model_comp = registry.try_get<ModelComponent>(entity))
		{
			return resource_manager.peek_animation_data(model_comp->model);
		}

		return {};
	}

	Entity World::get_bone_by_id(Entity entity, BoneID bone_id, bool recursive) const
	{
		if (!bone_id)
		{
			return null;
		}

		const auto relationship = registry.try_get<RelationshipComponent>(entity);

		if (!relationship)
		{
			return null;
		}

		const auto skeletal_comp = registry.try_get<SkeletalComponent>(entity);

		if (!skeletal_comp)
		{
			return null;
		}

		const auto animation_data = get_animation_data(entity);

		if (!animation_data)
		{
			return null;
		}

		const auto& skeleton = animation_data->skeleton;

		Entity bone_out = null;

		relationship->enumerate_children
		(
			registry,
			
			[&](Entity child, const RelationshipComponent& relationship, Entity next_child)
			{
				const auto bone_comp = registry.try_get<BoneComponent>(child);

				if (!bone_comp)
				{
					return true;
				}

				const auto bone = skeleton.get_bone_by_index(bone_comp->bone_index);

				if ((bone) && (bone->name == bone_id))
				{
					bone_out = child;

					return false;
				}
				else if (recursive)
				{
					const auto recursive_result = get_bone_by_id(child, bone_id, true);

					if (recursive_result != null)
					{
						bone_out = recursive_result;

						return false;
					}
				}

				return true;
			}
		);

		return bone_out;
	}

	Entity World::get_bone_by_name(Entity entity, std::string_view name, bool recursive) const
	{
		return get_bone_by_id(entity, hash(name), recursive);
	}

	Entity World::get_bone_by_index(Entity entity, BoneIndex bone_index, bool recursive) const
	{
		const auto relationship = registry.try_get<RelationshipComponent>(entity);

		if (!relationship)
		{
			return null;
		}

		Entity bone_out = null;

		relationship->enumerate_children
		(
			registry,
			
			[&](Entity child, const RelationshipComponent& relationship, Entity next_child)
			{
				const auto bone_comp = registry.try_get<BoneComponent>(child);

				if (!bone_comp)
				{
					return true;
				}

				if (bone_comp->bone_index == bone_index)
				{
					bone_out = child;

					return false;
				}
				else if (recursive)
				{
					const auto recursive_result = get_bone_by_index(child, bone_index, true);

					if (recursive_result != null)
					{
						bone_out = recursive_result;

						return false;
					}
				}

				return true;
			}
		);

		return bone_out;
	}

	ResourceManager& World::get_resource_manager()
	{
		return resource_manager;
	}

	const ResourceManager& World::get_resource_manager() const
	{
		return resource_manager;
	}

	// NOTE: Unsafe. (Assumes remote `DeltaSystem` object will always be valid)
	const DeltaSystem* World::get_delta_system() const
	{
		if (this->_delta_system)
		{
			return this->_delta_system;
		}

		if (const auto* delta_system = systems.get_system<DeltaSystem>())
		{
			//this->_delta_system = const_cast<DeltaSystem*>(delta_system);

			return delta_system;
		}

		return {};
	}

	// NOTE: Unsafe. (Assumes remote `DeltaSystem` object will always be valid)
	DeltaSystem* World::get_delta_system()
	{
		if (this->_delta_system)
		{
			return this->_delta_system;
		}

		if (auto* delta_system = systems.get_system<DeltaSystem>())
		{
			this->_delta_system = delta_system;
		}

		return {};
	}

	const DeltaTime& World::get_delta_time() const
	{
		auto delta_system = get_delta_system();

		assert(delta_system);

		return delta_system->get_delta_time();
	}
	
	float World::get_delta() const
	{
		if (auto delta_system = get_delta_system())
		{
			return delta_system->get_delta();
		}

		return 1.0f;
	}

	// The actively bound camera.
	// NOTE: Does not always represent the rendering camera.
	Entity World::get_camera() const
	{
		if (auto camera_system = systems.get_system<CameraSystem>())
		{
			return camera_system->get_active_camera();
		}

		return null;
	}

	void World::set_camera(Entity camera)
	{
		if (auto camera_system = systems.get_system<CameraSystem>())
		{
			camera_system->set_active_camera(camera);
		}
	}

	math::Vector World::get_gravity() const
	{
		return properties.gravity;
	}

	void World::set_gravity(const math::Vector& gravity)
	{
		// Temporarily store the old gravity vector.
		const auto old_gravity = properties.gravity;
		
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

	void World::on_transform_changed(const OnTransformChanged& tform_change)
	{
		const auto entity = tform_change.entity;

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
			name_label = " \"" + name_component->get_name() +"\"";
		}

		print("Entity #{}{} ({}) - Transform Changed: {}", entity, name_label, entity_type, get_transform(entity).get_vectors());
		*/
	}

	void World::handle_transform_events(float delta)
	{
		auto& registry = get_registry();

		// Another pass is required, in order to ensure all of the hierarchy has a chance to validate collision representation:
		registry.view<TransformComponent>().each([this](auto entity, auto& tf)
		{
			tf.on_flag(TransformComponent::Flag::EventFlag, [this, entity]()
			{
				//this->queue_event<OnTransformChanged>(entity);
				this->event<OnTransformChanged>(entity);
			});

			// Already handled by `on_flag`.
			//tf.validate(TransformComponent::Dirty::EventFlag);
		});

		/*
		registry.view<TransformComponent, RelationshipComponent>().each([&](auto entity, auto& tf, auto& rel)
		{
			auto transform = Transform(registry, entity, rel, tf);

			//transform.validate_collision();
			transform.validate_collision_shallow();
		});
		*/
	}

	std::size_t World::update_light_type(Entity entity, LightType light_type)
	{
		auto& light_comp = registry.get_or_emplace<LightComponent>(entity);

		return update_light_type(entity, light_type, light_comp);
	}

	std::size_t World::update_light_type(Entity entity, LightType light_type, LightComponent& light_comp)
	{
		std::size_t components_removed = 0;

		if (light_comp.type != LightType::Any)
		{
			if (light_comp.type != light_type)
			{
				light_comp.type = light_type;

				components_removed += remove_light_sub_components(registry, entity, light_type);
			}
		}

		// If we're currently indicating that we don't cast shadows,
		// double-check that this status is still valid:
		if (!light_comp.casts_shadows)
		{
			if (has_light_shadow_sub_components(entity))
			{
				light_comp.casts_shadows = true;
			}
		}

		// Check against the `casts_shadows` flag again,
		// since we could have modified it in the scenario above:
		if (light_comp.casts_shadows)
		{
			attach_shadows(entity, light_type);
		}

		return components_removed;
	}

	bool World::attach_shadows(Entity entity, LightType light_type)
	{
		return engine::attach_shadows
		(
			*this,
			
			entity,
			light_type,
			
			config.graphics.shadows.resolution,
			config.graphics.shadows.cubemap_resolution
		);
	}

	bool World::attach_shadows(Entity entity)
	{
		if (registry.try_get<DirectionalLightComponent>(entity))
		{
			if (!attach_shadows(entity, LightType::Directional))
			{
				return false;
			}
		}

		if (registry.try_get<PointLightComponent>(entity))
		{
			if (!attach_shadows(entity, LightType::Point))
			{
				return false;
			}
		}

		/*
		if (registry.try_get<SpotLightComponent>(entity))
		{
			if (!attach_shadows(entity, LightType::Spot))
			{
				return false;
			}
		}
		*/

		return true;
	}

	bool World::has_light_shadow_sub_components(Entity entity) const
	{
		return
		(
			registry.try_get<DirectionalLightShadowComponent>(entity)
			||
			registry.try_get<PointLightShadowComponent>(entity)
			//||
			//registry.try_get<SpotLightShadowComponent>(entity)
		);
	}

	void World::on_instance(Registry& registry, Entity entity)
	{
		auto& instance_comp = registry.get<InstanceComponent>(entity);

		const auto& descriptor = instance_comp.get_descriptor();

		if (!descriptor.model_details)
		{
			return;
		}

		if (registry.try_get<ModelComponent>(entity))
		{
			return;
		}

		load_model_attachment
		(
			*this,

			entity,

			descriptor.model_details.path,
			descriptor.model_details.allow_multiple,
			descriptor.model_details.collision_cfg
		);

		if (const auto& offset = descriptor.model_details.offset)
		{
			auto tform = get_transform(entity);

			tform.set_local_position(*offset);
		}
	}

	void World::on_light_init(Registry& registry, Entity entity)
	{
		auto& light_comp = registry.get<LightComponent>(entity);

		// NOTE: We don't need to explicitly validate `casts_shadows` here,
		// since any sub-component instances will already handle that step.
		ensure_light_sub_component(registry, entity, light_comp.type);
	}
	
	void World::on_light_update(Registry& registry, Entity entity)
	{
		auto& light_comp = registry.get<LightComponent>(entity);

		remove_light_sub_components(registry, entity, light_comp.type);
		ensure_light_sub_component(registry, entity, light_comp.type);

		// If we're not currently marked as casting shadows,
		// validate that there aren't any shadow-related sub-components:
		if (!light_comp.casts_shadows)
		{
			if (has_light_shadow_sub_components(entity))
			{
				// Shadow sub-component found, update the primary
				// component to indicate shadow-casting.
				light_comp.casts_shadows = true;
			}
		}

		// Check against the `casts_shadows` flag again,
		// since we could have modified it in the scenario above:
		if (light_comp.casts_shadows)
		{
			attach_shadows(entity);
		}
	}

	void World::on_light_destroyed(Registry& registry, Entity entity)
	{
		auto& light_comp = registry.get<LightComponent>(entity);

		// NOTE: Shadow-related sub-components are automatically
		// removed with their respective light components.
		remove_light_sub_components(registry, entity);
	}

	void World::on_directional_light_init(Registry& registry, Entity entity)
	{
		on_directional_light_update(registry, entity);
	}
	
	void World::on_directional_light_update(Registry& registry, Entity entity)
	{
		assert(&registry == &this->registry);

		update_light_type(entity, LightType::Directional);
	}
	
	void World::on_directional_light_destroyed(Registry& registry, Entity entity)
	{
		registry.remove<DirectionalLightShadowComponent>(entity);
	}
	
	void World::on_point_light_init(Registry& registry, Entity entity)
	{
		on_point_light_update(registry, entity);
	}
	
	void World::on_point_light_update(Registry& registry, Entity entity)
	{
		assert(&registry == &this->registry);

		update_light_type(entity, LightType::Point);
	}
	
	void World::on_point_light_destroyed(Registry& registry, Entity entity)
	{
		registry.remove<PointLightShadowComponent>(entity);
	}
	
	void World::on_spot_light_init(Registry& registry, Entity entity)
	{
		on_spot_light_update(registry, entity);
	}
	
	void World::on_spot_light_update(Registry& registry, Entity entity)
	{
		update_light_type(entity, LightType::Spot);
	}
	
	void World::on_spot_light_destroyed(Registry& registry, Entity entity)
	{
		//registry.remove<SpotLightShadowComponent>(entity);
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