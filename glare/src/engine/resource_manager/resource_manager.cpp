#include "resource_manager.hpp"

#include <graphics/model.hpp>
#include <graphics/shader.hpp>

#include <bullet/BulletCollision/btBulletCollisionCommon.h>

#include <memory>
#include <tuple>

#include <util/string.hpp>
#include <util/algorithm.hpp>

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
		: context(context) //, default_shader(default_shader)
	{
		set_default_shader(default_shader);
	}

	ResourceManager::~ResourceManager() {}

	ResourceManager::ModelData ResourceManager::load_model(const std::string& path, bool load_collision, pass_ref<graphics::Shader> shader, bool optimize_collision, bool force_reload, bool cache_result) const
	{
		auto path_resolved = resolve_path(path);

		if (shader)
		{
			force_reload = true;
			cache_result = false;
		}

		if (!force_reload)
		{
			auto lm_it = loaded_models.find(path_resolved);

			if (lm_it != loaded_models.end())
			{
				auto loaded_model = lm_it->second;

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

				return { std::move(loaded_model), nullptr };
			}
		}

		auto model_data = graphics::Model::Load(get_context(), path, ((shader) ? shader : get_default_shader()), load_collision);

		auto loaded_model = memory::allocate<graphics::Model>(); // ModelRef
		*loaded_model = std::move(std::get<0>(model_data));

		ASSERT(loaded_model->has_meshes());

		if (cache_result)
		{
			loaded_models[path_resolved] = loaded_model;
		}

		if (load_collision)
		{
			auto collision_opt = std::move(std::get<1>(model_data));

			if (collision_opt)
			{
				auto& ref_out = (collision_data[loaded_model] = { std::move(collision_opt.value()), optimize_collision }); // CollisionData col_out = ... ; // [loaded_model.get()]

				return { std::move(loaded_model), &ref_out };
			}
		}

		return { std::move(loaded_model), nullptr };
	}

	ResourceManager::ShaderRef ResourceManager::get_shader(const std::string& vertex_path, const std::string& fragment_path, bool force_reload, bool cache_result) const
	{
		auto vert_resolved = resolve_path(vertex_path);
		auto frag_resolved = resolve_path(fragment_path);

		auto shader_uid = util::concat(vertex_path, "|", fragment_path);

		if (!force_reload)
		{
			// TODO: Implement optimization for underlying shader-objects. (e.g. re-use vertex shader at link-time)
			auto lookup = util::get_if_exists<ShaderRef>(loaded_shaders, shader_uid);

			if (lookup)
			{
				return *lookup;
			}
		}

		auto shader = std::make_shared<graphics::Shader>(context, vertex_path, fragment_path);

		if (cache_result)
		{
			loaded_shaders[shader_uid] = shader;
		}

		return shader;
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