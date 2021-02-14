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
	ResourceManager::ResourceManager(pass_ref<graphics::Context> context, pass_ref<graphics::Shader> default_shader)
		: context(context), default_shader(default_shader)
	{}

	ResourceManager::~ResourceManager() {}

	ResourceManager::ModelData ResourceManager::load_model(const std::string& path, bool load_collision, bool optimize_collision) const
	{
		//ModelData loaded_model;

		auto model_data = graphics::Model::Load(get_context(), path, get_default_shader(), load_collision);

		ModelRef loaded_model;

		loaded_model = memory::allocate<graphics::Model>();
		*loaded_model = std::move(std::get<0>(model_data));

		loaded_models[path] = loaded_model;

		if (load_collision)
		{
			auto collision_opt = std::move(std::get<1>(model_data));

			if (collision_opt)
			{
				auto col_in = std::move(collision_opt.value());

				auto shape = std::make_shared<btBvhTriangleMeshShape>(col_in.mesh_interface.get(), optimize_collision); // std::shared_ptr<btTriangleMeshShape>

				auto& ref_out = (collision_data[loaded_model] = { std::static_pointer_cast<CollisionRaw>(shape), std::move(col_in) }); // CollisionData col_out = ... ; // [loaded_model.get()]

				return { std::move(loaded_model), &ref_out };
			}
		}

		return { std::move(loaded_model), nullptr };
	}

	ResourceManager::CollisionData ResourceManager::get_capsule_collision(float radius, float height) // ref<btCapsuleShape>
	{
		return { std::static_pointer_cast<CollisionRaw>(std::make_shared<btCapsuleShape>(radius, height)), std::nullopt };
	}

	std::string ResourceManager::resolve_path(const std::string& path)
	{
		// TODO: Implement logic to handle different paths to the same resource.
		return path;
	}
}