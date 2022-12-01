#include "resource_manager.hpp"
#include "collision_data.hpp"

#include <graphics/model.hpp>
#include <graphics/shader.hpp>

#include <bullet/btBulletCollisionCommon.h>

#include <memory>
#include <tuple>
#include <limits>

#include <math/bullet.hpp>

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
	CollisionData ResourceManager::generate_capsule_collision(float radius, float height)
	{
		return { std::static_pointer_cast<CollisionRaw>(std::make_shared<btCapsuleShape>(radius, height)) };
	}

	// TODO: Implement some form of caching/optimization for basic shapes.
	CollisionData ResourceManager::generate_sphere_collision(float radius)
	{
		return { std::static_pointer_cast<CollisionRaw>(std::make_shared<btSphereShape>(radius)) };
	}

	// TODO: Implement some form of caching/optimization for basic shapes.
	CollisionData ResourceManager::generate_cube_collision(float radius)
	{
		return generate_cube_collision({radius, radius, radius});
	}

	// TODO: Implement some form of caching/optimization for basic shapes.
	CollisionData ResourceManager::generate_cube_collision(const math::Vector& size)
	{
		return { std::static_pointer_cast<CollisionRaw>(std::make_shared<btBoxShape>(math::to_bullet_vector(size))) };
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

	const EntityFactoryData* ResourceManager::get_existing_factory(const std::string& path) const // std::string_view
	{
		auto it = entity_factories.find(path);

		if (it != entity_factories.end())
		{
			return &it->second;
		}

		return nullptr;
	}

	const EntityDescriptor* ResourceManager::get_existing_descriptor(const std::string& path) const
	{
		const auto* factory_data = get_existing_factory(path);

		//assert(factory_data);

		if (!factory_data)
		{
			return nullptr;
		}

		const auto& descriptor = factory_data->factory.get_descriptor();

		return &descriptor;
	}

	const EntityFactoryData* ResourceManager::get_factory(const EntityFactoryContext& context) const
	{
		auto path_str = context.paths.instance_path.string();

		if (auto* existing_factory = get_existing_factory(path_str))
		{
			return existing_factory;
		}

		EntityFactoryChildren children;

		auto factory = EntityFactory
		(
			context,

			[this, &path_str, &children](const EntityFactory& parent_factory, const EntityFactoryContext& child_context)
			{
				// NOTE: Recursion.
				auto child_factory_data = this->get_factory(child_context);

				assert(child_factory_data);

				if (!child_factory_data)
				{
					return;
				}

				children.push_back(child_context.paths.instance_path.string()); // c_str();
			}
		);

		if (auto [it, result] = entity_factories.emplace(path_str, EntityFactoryData { std::move(factory), std::move(children) }); result)
		{
			// ...

			return &it->second;
		}

		return nullptr;
	}

	Entity ResourceManager::generate_entity(const EntityFactoryContext& factory_context, const EntityConstructionContext& entity_context) const
	{
		auto* factory = get_factory(factory_context);

		if (factory)
		{
			return factory->create(entity_context);
		}

		return null;
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

	CollisionData ResourceManager::generate_shape(const util::json& collision_data)
	{
		using enum CollisionShapePrimitive;

		auto shape_primitive = collision_shape_primitive(util::get_value<std::string>(collision_data, "shape", "capsule"));

		switch (shape_primitive)
		{
			case Capsule:
			{
				auto radius = util::get_value<float>(collision_data, "radius", 2.0f);
				auto height = util::get_value<float>(collision_data, "height", 8.0f);

				return generate_capsule_collision(radius, height);
			}
			case Cube:
			{
				if (collision_data.contains("size"))
				{
					auto size = util::get_vector(collision_data, "size", { 4.0f, 4.0f, 4.0f });

					return generate_cube_collision(size);
				}
				
				auto radius = util::get_value<float>(collision_data, "radius", 4.0f);

				return generate_cube_collision(radius);
			}
			case Sphere:
			{
				auto radius = util::get_value<float>(collision_data, "radius", 4.0f);

				return generate_sphere_collision(radius);
			}

			default:
				// Unsupported shape primitive.
				//assert(false);

				break;
		}

		return {};
	}
}