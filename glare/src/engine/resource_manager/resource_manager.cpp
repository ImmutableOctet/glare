#include "resource_manager.hpp"
#include "collision_data.hpp"

#include <graphics/model.hpp>
#include <graphics/shader.hpp>

#include <bullet/btBulletCollisionCommon.h>

#include <memory>
#include <tuple>

#include <util/string.hpp>
#include <util/algorithm.hpp>

#include <engine/world/world.hpp>

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
	ResourceManager::ResourceManager(pass_ref<graphics::Context> context, pass_ref<graphics::Shader> default_shader, pass_ref<graphics::Shader> default_animated_shader)
		: context(context) //, default_shader(default_shader)
	{
		set_default_shader(default_shader);
		set_default_animated_shader(default_animated_shader);
	}

	ResourceManager::~ResourceManager() {}

	const ResourceManager::ModelData& ResourceManager::load_model(const std::string& path, bool load_collision, pass_ref<graphics::Shader> shader, bool optimize_collision, bool force_reload, bool cache_result) const
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
				return lm_it->second;
			}
		}

		auto model_loader = ModelLoader
		(
			get_context(),
			((shader) ? shader : get_default_shader()),
			((shader) ? shader : get_default_animated_shader()),
			{
				.maintain_storage  = false,
				//.is_animated     = true,
				.load_collision    = load_collision,
				.load_mesh_content = true
			}
		);

		ModelData model_data_out;

		ref<AnimationData> animations;

		model_loader.on_model = [&](ModelLoader& loader, ModelLoader::ModelData& model_data) -> void
		{
			auto loaded_model = memory::allocate<graphics::Model>(); // ModelRef
			*loaded_model = std::move(model_data.model);

			assert(loaded_model->has_meshes());

			if (load_collision)
			{
				if (model_data.collision.has_value())
				{
					collision_data[loaded_model] = { std::move(model_data.collision.value()), optimize_collision };
				}
			}

			if (loader.get_config().is_animated.value_or(false) || (loader.has_skeleton()))
			{
				if (!animations)
				{
					animations = memory::allocate<AnimationData>();
				}

				animation_data[loaded_model] = animations;
			}

			model_data_out.models.push_back
			(
				ModelData::ModelEntry
				{
					loaded_model, model_data.transform
				}
			);
		};

		model_loader.load(path);

		//auto& model_data = model_loader.get_model_storage();

		if (model_loader.has_animations() && (animations))
		{
			animations->animations = std::move(model_loader.get_animations());
		}

		if (model_loader.has_skeleton() && (animations))
		{
			animations->skeleton = std::move(model_loader.get_skeleton());
		}

		if (cache_result)
		{
			loaded_models[path_resolved] = std::move(model_data_out);
		}

		return loaded_models[path_resolved];
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

	// TODO: Implement some form of caching/optimization for basic shapes.
	CollisionData ResourceManager::generate_capsule_collision(float radius, float height) // ref<btCapsuleShape>
	{
		return { std::static_pointer_cast<CollisionRaw>(std::make_shared<btCapsuleShape>(radius, height)) };
	}

	// TODO: Implement some form of caching/optimization for basic shapes.
	CollisionData ResourceManager::generate_sphere_collision(float radius) // ref<btCapsuleShape>
	{
		return { std::static_pointer_cast<CollisionRaw>(std::make_shared<btSphereShape>(radius)) };
	}

	const CollisionData* ResourceManager::get_collision(const WeakModelRef model) const
	{
		auto it = collision_data.find(model);

		if (it != collision_data.end())
		{
			return &it->second;
		}

		return nullptr;
	}

	const ref<ResourceManager::AnimationData> ResourceManager::get_animation_data(const WeakModelRef model) const
	{
		auto it = animation_data.find(model);

		if (it != animation_data.end())
		{
			return it->second;
		}

		return nullptr;
	}

	void ResourceManager::subscribe(World& world)
	{
		//world.register_event<...>(*this);
	}

	std::string ResourceManager::resolve_path(const std::string& path)
	{
		// TODO: Implement logic to handle different paths to the same resource.
		return path;
	}
}