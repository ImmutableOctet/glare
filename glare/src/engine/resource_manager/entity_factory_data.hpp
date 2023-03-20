#pragma once

#include <engine/types.hpp>

#include <engine/entity/entity_factory.hpp>
#include <engine/entity/entity_construction_context.hpp>

#include <util/small_vector.hpp>

#include <memory>

namespace engine
{
	struct EntityConstructionContext;

	struct EntityFactoryData;

	using EntityFactoryChildren = util::small_vector<std::shared_ptr<const EntityFactoryData>, 8>; // 4

	struct EntityFactoryData
	{
		public:
			using Children = EntityFactoryChildren;

			// The underlying factory instance.
			EntityFactory factory;

			// List of factory instance paths, representing children of entities
			// produced by `create` and related functions.
			Children children;

			static Entity create(std::shared_ptr<const EntityFactoryData> factory_data, const EntityConstructionContext& context, bool handle_children=true);

			// Generates the child entities associated with this factory, then adds them as children to `entity`.
			// Entities are generated from referenced factories (via instance-path) -- i.e. `EntityFactoryData` instances.
			// 
			// NOTE: If `parent` is `null` or omitted, this will attempt to use the `parent` field of `context`.
			bool generate_children(const EntityConstructionContext& child_context, Entity parent) const;

			// Generates the child entities associated with this factory, then adds them as children to `context.parent`.
			bool generate_children(const EntityConstructionContext& child_context) const;
		protected:
			// Generates an entity using `context`.
			Entity create_impl(std::shared_ptr<const EntityFactoryData>&& factory_data, const EntityConstructionContext& context, bool handle_children=true) const;
			
			Entity on_entity_create(Entity entity, const EntityConstructionContext& context) const;
	};
}