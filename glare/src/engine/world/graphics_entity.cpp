#include "graphics_entity.hpp"
#include "world.hpp"

#include <tuple>

#include <engine/resource_manager/resource_manager.hpp>
#include <engine/model_component.hpp>
#include <engine/bone_component.hpp>
#include <engine/world/animation.hpp>
#include <engine/relationship.hpp>

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
		auto entity = create_entity(world, parent, type);

		return attach_model(world, entity, model);
	}

	Entity attach_model(World& world, Entity entity, pass_ref<graphics::Model> model, graphics::ColorRGBA color)
	{
		///ASSERT(model);

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
		auto& resource_manager = world.get_resource_manager();

		const auto& model_data = resource_manager.load_model(path, collision_enabled, shader);

		if (model_data.models.empty())
		{
			return null;
		}

		const auto animation_data = resource_manager.get_animation_data(model_data.models[0]);

		auto& registry = world.get_registry();

		auto create_skeleton = [&](Entity skeletal_parent)
		{
			if (!animation_data)
			{
				return;
			}

			// Create all bones in the skeleton first:
			for (const auto& bone_entry : animation_data->skeleton.bones)
			{
				const auto& bone_name = bone_entry.first;
				const auto& bone = bone_entry.second;

				create_bone(world, bone.id, bone.node_transform, bone.offset, bone_name, skeletal_parent);
			}

			// Reassign bones to their parents, if known:
			auto& relationship = registry.get<Relationship>(skeletal_parent);

			relationship.enumerate_children(registry, [&](Entity child, Relationship& relationship, Entity next_child)
			{
				const auto* bone = registry.try_get<BoneComponent>(child);

				if (!bone)
					return true;

				const auto& parent_name = animation_data->skeleton.bones[bone->name].parent_name;

				auto new_parent = world.get_bone_by_name(skeletal_parent, parent_name, false);

				if (new_parent == null)
					return true;

				world.set_parent(child, new_parent, true); // false

				//auto tform = world.get_transform(child);

				//tform.set_matrix(offset);
				//tform.set_local_matrix(bone->offset);

				return true;
			});
		};

		auto process_model = [&](ref<graphics::Model> model, Entity parent) -> Entity
		{
			auto entity = create_model(world, model, parent, type);

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
					attach_animator(world, entity, animation_data, 0.01f);
				}
			}

			return entity;
		};

		Entity entity = null;

		if (allow_multiple)
		{
			//ASSERT(!model_data.models.empty());

			entity = engine::create_pivot(world, parent);

			if (model_data.models.empty())
			{
				ASSERT(false); // <-- Debugging related.

				return entity;
			}

			for (const auto& model : model_data.models)
			{
				process_model(model, entity);
			}
		}
		else
		{
			entity = process_model(model_data.models[0], parent);
		}
		
		return entity;
	}

	Entity attach_animator(World& world, Entity entity, const pass_ref<AnimationData> animations, float rate)
	{
		auto& registry = world.get_registry();

		const Animation* starting_animation = nullptr;

		if (!animations->animations.empty())
		{
			starting_animation = &animations->animations[0];
		}

		registry.emplace_or_replace<Animator>(entity, animations, starting_animation, rate);

		return entity;
	}
	
	Entity create_bone(World& world, BoneID bone_id, const math::Matrix& local_transform, const math::Matrix& offset, std::string_view bone_name, Entity parent, EntityType type)
	{
		auto& registry = world.get_registry();

		auto entity = engine::create_pivot(world, parent, type);
		//auto entity = engine::load_model(world, "assets/geometry/cone.b3d", parent, type, false);

		registry.emplace<BoneComponent>(entity, bone_id, std::string(bone_name), offset); // offset;

		{
			auto tform = world.get_transform(entity);

			tform.set_local_matrix(local_transform); // set_matrix(...)
		}

		// Debugging related:
		//auto dbg = engine::load_model(world, "assets/geometry/sphere.b3d", entity, type, false);
		//auto dbg = engine::load_model(world, "assets/geometry/cube.b3d", entity, type, false);
		auto dbg = engine::load_model(world, "assets/geometry/cone.b3d", entity, type, false);
		//auto dbg = entity;

		/*
		{
			auto dbg_tform = world.get_transform(dbg);

			dbg_tform.set_local_matrix(offset);
		}
		*/

		// Debugging related:
		auto* model_comp = registry.try_get<ModelComponent>(dbg);

		if (model_comp)
		{
			if (bone_id == 1)
			{
				model_comp->color = graphics::ColorRGBA{ 1.0f, 1.0f, 1.0f, 1.0f };
			}
			else
			{
				switch ((bone_id % 3) + 1)
				{
					case 1:
						model_comp->color = graphics::ColorRGBA{ 0.0f, 0.0f, 1.0f, 1.0f }; break;
					case 2:
						model_comp->color = graphics::ColorRGBA{ 0.0f, 1.0f, 0.0f, 1.0f }; break;
					case 3:
						model_comp->color = graphics::ColorRGBA{ 1.0f, 0.0f, 0.0f, 1.0f }; break;
				}
			}
		}

		return entity;
	}
}