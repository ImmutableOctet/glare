#pragma once

#include <engine/types.hpp>
#include <engine/entity/entity_factory.hpp>

#include <util/small_vector.hpp>
//#include <string>

namespace engine
{
	using EntityFactoryKey = EntityFactory::FactoryKey; // std::string; // std::filsystem::path;
	using EntityFactoryChildren = util::small_vector<EntityFactoryKey, 8>; // 16

	struct EntityFactoryData
	{
		using FactoryKey = EntityFactoryKey;

		// The underlying factory instance.
		EntityFactory factory;

		// List of factory instance paths, representing children of entities
		// produced by `create` and related functions.
		EntityFactoryChildren children;

		// Generates an entity using `context`.
		Entity create(const EntityConstructionContext& context, bool handle_children=true) const;

		// Generates the child entities associated with this factory, then adds them as children to `entity`.
		// Entities are generated from referenced factories (via instance-path) -- i.e. `EntityFactoryData` instances.
		// 
		// NOTE: If `parent` is `null` or omitted, this will attempt to use the `parent` field of `context`.
		bool generate_children(const EntityConstructionContext& child_context, Entity parent) const;

		// Generates the child entities associated with this factory, then adds them as children to `context.parent`.
		bool generate_children(const EntityConstructionContext& child_context) const;
	};
}