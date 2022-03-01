#include "graphics_entity.hpp"
#include "world.hpp"

#include <tuple>
#include <utility>

#include <engine/resource_manager/resource_manager.hpp>
#include <engine/model_component.hpp>
#include <engine/name_component.hpp>
#include <engine/bone_component.hpp>
#include <engine/world/animation.hpp>
#include <engine/relationship.hpp>

#include <graphics/model.hpp>
#include <graphics/shader.hpp>
#include <graphics/skeleton.hpp>

namespace graphics
{
	//class Model;
	class Context;
}

namespace engine
{
	Entity create_model(World& world, pass_ref<graphics::Model> model, Entity parent, EntityType type)
	{
		ASSERT(model);

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
		///ASSERT(model);

		auto& registry = world.get_registry();

		registry.emplace_or_replace<ModelComponent>(entity, model, color);
		//registry.emplace_or_replace<RenderFlagsComponent>(entity);

		return entity;
	}

	static std::tuple<bool, Entity> build_ligaments(World& world, Entity parent, const graphics::Skeleton& skeleton, const std::pair<std::string, graphics::Bone>& bone_entry, bool generate_parent=true, bool check_for_existing=true)
	{
		const auto& bone_name = bone_entry.first;
		const auto& bone_data = bone_entry.second;

		const auto& parent_name = bone_data.parent_name;

		if (generate_parent)
		{
			if (!parent_name.empty())
			{
				auto parent_it = skeleton.bones.find(parent_name);

				if (parent_it != skeleton.bones.end())
				{
					auto new_parent = std::get<1>(build_ligaments(world, parent, skeleton, *parent_it, true, true));

					if (new_parent != null)
					{
						parent = new_parent;
					}
				}
			}
		}

		if (check_for_existing)
		{
			auto existing = world.get_bone_by_name(parent, bone_name, true); // false

			if (existing != null)
			{
				return { false, existing };
			}
		}

		return { true, create_bone(world, bone_data, bone_name, parent) };
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

			// Debugging related:
			registry.emplace_or_replace<NameComponent>(entity, std::format("{} - {}", path, model->get_name()));

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
					attach_animator(world, entity, animation_data, 0.01f); // 0.01f
				}
			}

			return entity;
		};

		Entity entity = null;

		if (allow_multiple && (model_data.models.size() > 1))
		{
			//ASSERT(!model_data.models.empty());

			entity = engine::create_pivot(world, parent);

			for (const auto& model : model_data.models)
			{
				process_model(model, entity);
			}
		}
		else
		{
			if (model_data.models.empty())
			{
				ASSERT(false); // <-- Debugging related.

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

		// TODO: Look into optimizing out BoneComponent's `name` field.
		registry.emplace<NameComponent>(entity, std::string(bone_name));

		registry.emplace<BoneComponent>(entity, bone_id, std::string(bone_name), offset); // offset;

		{
			auto tform = world.get_transform(entity);

			tform.set_local_matrix(local_transform); // set_matrix(...)
		}

		// Debugging related:
		auto dbg_parent = entity; // null;

		auto dbg = engine::load_model(world, "assets/geometry/directions_small.b3d", dbg_parent, type, false);
		//auto dbg = engine::load_model(world, "assets/geometry/cube.b3d", dbg_parent, type, false);

		//auto dbg = engine::load_model(world, "assets/geometry/sphere.b3d", dbg_parent, type, false);
		//auto dbg = engine::load_model(world, "assets/geometry/cone.b3d", dbg_parent, type, false);

		//auto dbg = entity;

		/*
		{
			auto dbg_tform = world.get_transform(dbg);

			dbg_tform.set_local_matrix(offset);
			//dbg_tform.set_local_matrix(local_transform);
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

	Entity create_bone(World& world, const graphics::Bone& bone_data, std::string_view bone_name, Entity parent, EntityType type)
	{
		return create_bone(world, bone_data.id, bone_data.node_transform, bone_data.offset, bone_name, parent, type);
	}
}