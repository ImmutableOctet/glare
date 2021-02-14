#include "resource_manager.hpp"

#include <graphics/model.hpp>

#include <bullet/BulletCollision/btBulletCollisionCommon.h>

#include <memory>
#include <tuple>

/*
#define AI_CONFIG_PP_PTV_NORMALIZE   "PP_PTV_NORMALIZE"

#include <assimp/Importer.hpp>
#include <assimp/vector3.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/material.h>

// TODO: Refactor model loader as subroutine of 'ResourceManager' class.
*/

namespace engine
{
	ResourceManager::CollisionData::CollisionData(const Shape& collision_shape)
		: collision_shape(collision_shape) {}

	ResourceManager::CollisionData::CollisionData(Geometry&& geometry_storage, bool optimize)
		: collision_shape(ResourceManager::build_mesh_shape(geometry_storage, optimize)), geometry_storage(std::move(geometry_storage)) {}

	ResourceManager::ResourceManager(pass_ref<graphics::Context> context, pass_ref<graphics::Shader> default_shader)
		: context(context), default_shader(default_shader)
	{}

	ResourceManager::~ResourceManager() {}

	ResourceManager::ModelData ResourceManager::load_model(const std::string& path, bool load_collision, bool optimize_collision) const
	{
		auto path_resolved = resolve_path(path);

		//ModelData loaded_model;

		auto model_data = graphics::Model::Load(get_context(), path, get_default_shader(), load_collision);

		ModelRef loaded_model;

		auto lm_it = loaded_models.find(path_resolved);

		if (lm_it != loaded_models.end())
		{
			loaded_model = lm_it->second;

			if (load_collision)
			{
				auto cd_it = collision_data.find(loaded_model);

				if (cd_it == collision_data.end())
				{
					// TODO: INSERT CODE TO LOAD COLLISION SEPARATELY HERE.
					ASSERT(false);
				}
				else
				{
					return { std::move(loaded_model), &cd_it->second };
				}
			}
		}
		else
		{
			loaded_model = memory::allocate<graphics::Model>();
			*loaded_model = std::move(std::get<0>(model_data));

			ASSERT(loaded_model->has_meshes());

			loaded_models[path_resolved] = loaded_model;

			if (load_collision)
			{
				auto collision_opt = std::move(std::get<1>(model_data));

				if (collision_opt)
				{
					auto& ref_out = (collision_data[loaded_model] = { std::move(collision_opt.value()), optimize_collision }); // CollisionData col_out = ... ; // [loaded_model.get()]

					return { std::move(loaded_model), &ref_out };
				}
			}
		}

		return { std::move(loaded_model), nullptr };
	}

	ResourceManager::CollisionData ResourceManager::get_capsule_collision(float radius, float height) // ref<btCapsuleShape>
	{
		return { std::static_pointer_cast<CollisionRaw>(std::make_shared<btCapsuleShape>(radius, height)) };
	}

	std::string ResourceManager::resolve_path(const std::string& path)
	{
		// TODO: Implement logic to handle different paths to the same resource.
		return path;
	}

	ResourceManager::CollisionShape ResourceManager::build_mesh_shape(const ResourceManager::CollisionGeometry& geometry_storage, bool optimize)
	{
		auto desc = geometry_storage.mesh_interface.get(); // auto*

		ASSERT(desc);

		auto shape = std::make_shared<btBvhTriangleMeshShape>(desc, optimize); // std::shared_ptr<btTriangleMeshShape>

		return std::static_pointer_cast<CollisionRaw>(shape);
	}
}