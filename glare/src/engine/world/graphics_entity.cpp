#include "graphics_entity.hpp"
#include "world.hpp"

#include <tuple>
#include <utility>

#include "animation/animation.hpp"
#include "animation/skeletal_component.hpp"
#include "animation/bone_component.hpp"

//#include "physics/collision_config.hpp"
#include "physics/collision_component.hpp"

#include <engine/resource_manager/resource_manager.hpp>

#include <engine/model_component.hpp>
#include <engine/name_component.hpp>

#include <graphics/model.hpp>
#include <graphics/shader.hpp>

namespace graphics
{
	//class Model;
	class Context;
}

namespace engine
{
	Entity create_model(World& world, pass_ref<graphics::Model> model, Entity parent, EntityType type)
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

	Entity attach_model(World& world, Entity entity, pass_ref<graphics::Model> model, graphics::ColorRGBA color)
	{
		///assert(model);

		auto& registry = world.get_registry();

		registry.emplace_or_replace<ModelComponent>(entity, model, color);
		//registry.emplace_or_replace<RenderFlagsComponent>(entity);

		return entity;
	}

	Entity load_model
	(
		World& world, const std::string& path, Entity parent, EntityType type,

		bool allow_multiple,
		bool collision_enabled, float mass,

		std::optional<CollisionGroup> collision_group,
		std::optional<CollisionGroup> collision_solid_mask,
		std::optional<CollisionGroup> collision_interaction_mask,

		pass_ref<graphics::Shader> shader
	)
	{
		if (path.empty())
		{
			return null;
		}

		auto& resource_manager = world.get_resource_manager();

		const auto& model_data = resource_manager.load_model(path, collision_enabled, shader);

		if (model_data.models.empty())
		{
			return null;
		}

		const auto animation_data = resource_manager.get_animation_data(model_data.models[0].model);

		auto& registry = world.get_registry();

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

		auto process_model = [&](ModelData::ModelEntry model_entry, Entity parent) -> Entity
		{
			auto& model = model_entry.model;

			auto entity = create_model(world, model, parent, type);

			//world.set_name(entity, std::format("{} - {}", path, model->get_name()));
			world.set_name(entity, model->get_name());

			// Update model's scene-local transform:
			{
				auto tform = world.get_transform(entity);

				tform.set_local_matrix(model_entry.transform);
				//tform.set_matrix(model_entry.transform);
			}

			if (collision_enabled)
			{
				const auto* collision_data = resource_manager.get_collision(model); // auto*

				if (collision_data)
				{
					attach_collision(world, entity, collision_data->collision_shape, CollisionConfig(type), mass);
				}
			}

			if (animation_data)
			{
				if (!animation_data->animations.empty())
				{
					create_skeleton(entity);
					attach_animator(world, entity, animation_data, 0.05f); // 0.01f
				}
			}

			return entity;
		};

		Entity entity = null;

		if (allow_multiple && (model_data.models.size() > 1))
		{
			//assert(!model_data.models.empty());

			entity = engine::create_pivot(world, parent);
			world.set_name(entity, path);

			for (const auto& model : model_data.models)
			{
				process_model(model, entity);
			}
		}
		else
		{
			if (model_data.models.empty())
			{
				assert(false); // <-- Debugging related.

				return entity;
			}

			entity = process_model(model_data.models[0], parent);
		}
		
		return entity;
	}

	Entity create_cube(World& world, Entity parent, EntityType type)
	{
		return load_model(world, "assets/geometry/cube.b3d", parent, type, false);
	}

	Entity attach_animator(World& world, Entity entity, const pass_ref<AnimationData> animations, float rate)
	{
		auto& registry = world.get_registry();

		assert(!animations->animations.empty());

		registry.emplace_or_replace<Animator>(entity, animations, AnimationID(0), rate);

		return entity;
	}
}