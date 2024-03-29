#include "graphics_entity.hpp"
#include "world.hpp"

#include <tuple>
#include <utility>

#include "animation/components/animation_component.hpp"
#include "animation/components/skeletal_component.hpp"
#include "animation/components/bone_component.hpp"

#include "physics/components/collision_component.hpp"

#include <engine/resource_manager/resource_manager.hpp>
#include <engine/resource_manager/animation_data.hpp>

#include <engine/components/model_component.hpp>
#include <engine/components/name_component.hpp>

#include <graphics/model.hpp>
#include <graphics/shader.hpp>

namespace graphics
{
	//class Model;
	class Context;
}

namespace engine
{
	// TODO: Remove/deprecate.
	template <typename ShapeType>
	static Entity attach_collision
	(
		World& world, Entity entity,
		const ShapeType& collision_shape_data,
		const CollisionConfig& config,
		float mass=0.0f,
		std::optional<KinematicResolutionConfig> resolution_method=std::nullopt
	) // CollisionComponent::Shape
	{
		// TODO: Refactor into something more explicit from the user. (or something better for automatically determining body type)
		// We currently assume kinematic rigid bodies if a motion-state is generated:
		std::unique_ptr<CollisionMotionState> motion_state;

		CollisionBodyType body_type = get_collision_body_type(config.group);

		switch (body_type)
		{
			//case CollisionBodyType::Kinematic:
			case CollisionBodyType::Dynamic:
				motion_state = make_collision_motion_state(world, entity, config);

				break;
		}

		auto& registry = world.get_registry();

		registry.emplace<CollisionComponent>
		(
			entity,

			collision_shape_data, config,
			resolution_method, body_type,
			mass, std::move(motion_state)
		);

		return entity;
	}

	Entity create_model(World& world, const std::shared_ptr<graphics::Model>& model, Entity parent, EntityType type)
	{
		assert(model);

		auto& registry = world.get_registry();

		auto entity = create_entity(world, parent, type);

		if (model->has_name())
		{
			registry.emplace<NameComponent>(entity, model->get_name());
		}

		return attach_model(world, entity, model);
	}

	Entity attach_model(World& world, Entity entity, const std::shared_ptr<graphics::Model>& model, graphics::ColorRGBA color, std::optional<bool> update_name)
	{
		///assert(model);

		auto& registry = world.get_registry();

		registry.emplace_or_replace<ModelComponent>(entity, model, color);
		//registry.emplace_or_replace<RenderFlagsComponent>(entity);

		if (model->has_name())
		{
			if (update_name.has_value())
			{
				if (*update_name)
				{
					world.set_name(entity, model->get_name());
				}
			}
			else
			{
				if (!world.has_name(entity))
				{
					world.set_name(entity, model->get_name());
				}
			}
		}

		return entity;
	}

	//entity = create_model(world, model, parent, type);

	Entity load_model_attachment
	(
		World& world, Entity entity,
		const std::string& path, // std::string_view path,

		bool allow_multiple,

		std::optional<CollisionConfig> collision_cfg,
		float mass,

		const std::shared_ptr<graphics::Shader>& shader
	)
	{
		if (path.empty())
		{
			return entity;
		}

		auto& registry = world.get_registry();
		auto& resource_manager = world.get_resource_manager();

		bool collision_enabled = false;

		if (collision_cfg.has_value())
		{
			collision_enabled = collision_cfg->enabled();
		}

		const auto& model_data = resource_manager.load_model(path, collision_enabled, shader);

		if (model_data.models.empty())
		{
			return null;
		}

		const auto animation_data = resource_manager.get_animation_data(model_data.models[0].model);

		auto create_skeleton = [&](Entity skinned_entity)
		{
			if (!animation_data)
			{
				return;
			}

			print("Generating skeleton for {}...", world.label(skinned_entity));

			const auto& skeleton = animation_data->skeleton;

			// Create all bones in the skeleton first:
			for (const auto& bone_entry : skeleton.bones)
			{
				build_ligaments(world, skinned_entity, skeleton, bone_entry);
			}

			print("Skeleton generated.");
		};

		auto process_model = [&](ModelData::ModelEntry model_entry, Entity entity) -> Entity
		{
			auto& model = model_entry.model;

			assert(entity != null);
			
			entity = attach_model(world, entity, model);

			// Update model's scene-local transform:
			{
				auto tform = world.get_transform(entity);

				tform.set_local_matrix(model_entry.transform); // tform.set_matrix(...);
			}

			if (collision_enabled) // && (collision_cfg.has_value())
			{
				const auto* collision_data = resource_manager.get_collision(model);

				if (collision_data)
				{
					attach_collision
					(
						world, entity,
						collision_data->collision_shape,
						*collision_cfg,
						mass
					);
				}
			}

			if (animation_data)
			{
				if (animation_data->skeleton)
				{
					create_skeleton(entity);
				}

				if (animation_data->frames)
				{
					registry.emplace_or_replace<AnimationComponent>(entity);
				}
			}

			return entity;
		};

		if (allow_multiple && (model_data.models.size() > 1))
		{
			//assert(!model_data.models.empty());

			for (const auto& model : model_data.models)
			{
				auto child = engine::create_pivot(world, entity, EntityType::Geometry);

				process_model(model, child);
			}
		}
		else
		{
			if (model_data.models.empty())
			{
				// Debugging related.
				assert(false);

				return entity;
			}

			entity = process_model(model_data.models[0], entity);
		}
		
		return entity;
	}

	Entity load_model
	(
		World& world,
		const std::string& path, // std::string_view path,

		Entity parent,
		EntityType type,

		bool allow_multiple,

		std::optional<CollisionConfig> collision_cfg,
		float mass,

		const std::shared_ptr<graphics::Shader>& shader
	)
	{
		if (path.empty())
		{
			return null;
		}

		auto& registry = world.get_registry();
		
		auto entity = engine::create_pivot(world, parent, type);
		
		world.set_name(entity, path);
		
		return load_model_attachment
		(
			world, entity, path, allow_multiple,
			collision_cfg,
			mass,
			shader
		);
	}

	Entity create_cube(World& world, Entity parent, EntityType type)
	{
		return load_model(world, "assets/geometry/cube.b3d", parent, type, false);
	}
}