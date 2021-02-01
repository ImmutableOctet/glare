#include "resource_manager.hpp"

#include <graphics/model.hpp>

#include <bullet/BulletCollision/btBulletCollisionCommon.h>

#include <memory>

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

	ResourceManager::ModelData ResourceManager::load_model(const std::string& path) const
	{
		ModelData loaded_model;

		loaded_model = memory::allocate<graphics::Model>();
		*loaded_model = graphics::Model::Load(get_context(), path, get_default_shader());

		return loaded_model;
	}

	ResourceManager::CollisionData ResourceManager::get_capsule_collision(float radius, float height) // ref<btCapsuleShape>
	{
		return std::static_pointer_cast<CollisionRaw>(std::make_shared<btCapsuleShape>(radius, height));
	}
}