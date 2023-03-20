#include "resource_manager.hpp"

#include "animation_data.hpp"
#include "entity_factory_data.hpp"

#include "loaders/loaders.hpp"

#include <engine/meta/meta_type_resolution_context.hpp>

#include <engine/entity/entity_factory_context.hpp>
#include <engine/entity/entity_construction_context.hpp>

#include <engine/world/physics/collision_shape_description.hpp>
#include <engine/world/world.hpp>

#include <util/string.hpp>
#include <util/algorithm.hpp>

#include <math/bullet.hpp>

#include <graphics/model.hpp>
#include <graphics/shader.hpp>

#include <bullet/btBulletCollisionCommon.h>

#include <memory>
#include <tuple>
#include <limits>

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
	ResourceManager::ResourceManager(const std::shared_ptr<graphics::Context>& context, const std::shared_ptr<graphics::Shader>& default_shader, const std::shared_ptr<graphics::Shader>& default_animated_shader)
		: context(context) //, default_shader(default_shader)
	{
		set_default_shader(default_shader);
		set_default_animated_shader(default_animated_shader);
	}

	ResourceManager::~ResourceManager() {}

	const ModelData& ResourceManager::load_model(const std::string& path, bool load_collision, const std::shared_ptr<graphics::Shader>& shader, bool optimize_collision, bool force_reload, bool cache_result) const
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

		std::shared_ptr<AnimationData> animations;

		model_loader.on_model = [&](ModelLoader& loader, ModelLoader::ModelData& model_data) -> void
		{
			auto loaded_model = std::make_shared<graphics::Model>(); // ModelRef
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
					animations = std::make_shared<AnimationData>();
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

	const CollisionData* ResourceManager::get_collision(const WeakModelRef model) const
	{
		auto it = collision_data.find(model);

		if (it != collision_data.end())
		{
			return &it->second;
		}

		return nullptr;
	}

	const std::shared_ptr<AnimationData> ResourceManager::get_animation_data(const WeakModelRef model) const
	{
		auto it = animation_data.find(model);

		if (it != animation_data.end())
		{
			return it->second;
		}

		return nullptr;
	}

	// NOTE: Implementation may not be thread-safe due to use of `erase`.
	std::shared_ptr<const EntityFactoryData> ResourceManager::get_existing_factory(const std::string& path) const // std::string_view
	{
		auto it = entity_factories.find(path);

		if (it != entity_factories.end())
		{
			if (auto ptr_out = it->second.lock())
			{
				return ptr_out;
			}
			else
			{
				entity_factories.erase(it);
			}
		}

		return {};
	}

	// NOTE: We may want to revisit this interface, since `factory_data` could theoretically become
	// the only reference to the underlying factory data during the execution of this function.
	// (This is very unlikely in practice, but still possible)
	const EntityDescriptor* ResourceManager::get_existing_descriptor(const std::string& path) const
	{
		auto factory_data = get_existing_factory(path);

		//assert(factory_data);

		if (!factory_data)
		{
			return {};
		}

		const auto& descriptor = factory_data->factory.get_descriptor();

		return &descriptor;
	}

	std::shared_ptr<const EntityFactoryData> ResourceManager::get_factory(const EntityFactoryContext& context) const
	{
		const auto path_str = context.paths.instance_path.string();

		if (auto existing_factory = get_existing_factory(path_str))
		{
			return existing_factory;
		}

		// List of factory paths representing children of this entity.
		// (Used during entity instantiation)
		EntityFactoryChildren children;

		const auto& command_context = get_parsing_context();

		auto factory = EntityFactory
		(
			context,

			// NOTE: Recursion.
			[this, &children](const EntityDescriptor& parent_descriptor, const EntityFactoryContext& child_context)
			{
				// Call back into this function recursively for each child.
				auto child_factory_data = this->get_factory(child_context);

				assert(child_factory_data);

				if (!child_factory_data)
				{
					return;
				}

				// Store the path of this child so that we can instantiate from `child_factory_data`
				// via lookup when `EntityFactoryData::create` is later called.
				// 
				// See also: `generate_entity`
				children.push_back(std::move(child_factory_data)); // child_context.paths.instance_path.string();
			},

			command_context
		);

		// NOTE: In the case of child factories, the parent factory is initialized last.
		// This applies recursively to each level of the hierarchy.
		auto factory_data = std::make_shared<EntityFactoryData>(std::move(factory), std::move(children));

		if (auto [it, result] = entity_factories.try_emplace(path_str, factory_data); result)
		{
			return factory_data;
		}

		return {}; // factory_data;
	}

	Entity ResourceManager::generate_entity(const EntityFactoryContext& factory_context, const EntityConstructionContext& entity_context, bool handle_children) const
	{
		if (auto factory = get_factory(factory_context))
		{
			return EntityFactoryData::create(std::move(factory), entity_context, handle_children);
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

	MetaParsingContext ResourceManager::set_type_resolution_context(MetaTypeResolutionContext&& context)
	{
		type_resolution_context = std::move(context);

		return get_existing_parsing_context();
	}

	MetaParsingContext ResourceManager::get_parsing_context() const
	{
		if (!type_resolution_context)
		{
			type_resolution_context = MetaTypeResolutionContext::generate();
		}

		return get_existing_parsing_context();
	}

	MetaParsingContext ResourceManager::get_existing_parsing_context() const
	{
		return
		{
			(type_resolution_context)
				? &(*type_resolution_context)
				: nullptr,

			{}
		};
	}
}