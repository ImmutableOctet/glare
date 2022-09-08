#include "bone_component.hpp"
#include "skeletal_component.hpp"

#include <engine/relationship.hpp>
#include <engine/name_component.hpp>

#include <engine/world/world.hpp>

// Debugging related:
#include <engine/world/graphics_entity.hpp>

namespace engine
{
	std::tuple<bool, Entity> build_ligaments(World& world, Entity parent, const graphics::Skeleton& skeleton, const std::pair<std::string, graphics::Bone>& bone_entry, bool generate_parent, bool check_for_existing)
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

	Entity create_bone(World& world, BoneID bone_id, const math::Matrix& local_transform, const math::Matrix& offset, std::string_view bone_name, Entity parent, EntityType type)
	{
		auto& registry = world.get_registry();

		auto entity = engine::create_pivot(world, parent, type);
		//auto entity = engine::load_model(world, "assets/geometry/cone.b3d", parent, type, false);

		// TODO: Look into optimizing out BoneComponent's `name` field.
		registry.emplace<NameComponent>(entity, std::string(bone_name));

		BoneComponent* parent_bone_component = nullptr;

		Entity skeleton = null;

		if (parent != null)
		{
			// Check if the parent entity is also a bone:
			parent_bone_component = registry.try_get<BoneComponent>(parent);

			if (parent_bone_component)
			{
				// Forward the `skeleton` field from the parent entity.
				// (Points to top-most parent in chain; see below)
				skeleton = parent_bone_component->skeleton;
			}
			else
			{
				// This parent entity isn't a bone, attach a skeleton to it.
				// NOTE: The root bone will be set to `entity` if no skeleton existed previously.
				attach_skeleton(world, parent, entity);

				// Set our skeleton instance to `parent` regardless of root/nested status.
				skeleton = parent;
			}
		}

		registry.emplace<BoneComponent>(entity, skeleton, bone_id, std::string(bone_name), offset); // offset;

		{
			auto tform = world.get_transform(entity);

			tform.set_local_matrix(local_transform); // set_matrix(...)
		}

		// Debugging related:
		auto dbg_parent = entity; // null;

		auto dbg = engine::load_model(world, "assets/geometry/directions.b3d", dbg_parent, type, false);
		//auto dbg = engine::load_model(world, "assets/geometry/cube.b3d", dbg_parent, type, false);

		auto* dbg_name = registry.try_get<NameComponent>(dbg);

		if (dbg_name)
		{
			dbg_name->name = "Debug"; // "Debug Orientation";
		}

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
		/*
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
		*/

		return entity;
	}

	Entity create_bone(World& world, const graphics::Bone& bone_data, std::string_view bone_name, Entity parent, EntityType type)
	{
		return create_bone(world, bone_data.id, bone_data.node_transform, bone_data.offset, bone_name, parent, type);
	}
}