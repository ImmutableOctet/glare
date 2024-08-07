#pragma once

#include "types.hpp"

#include "entity_factory_context.hpp"
#include "entity_descriptor.hpp"
#include "serial.hpp"

#include <util/json.hpp>
#include <util/algorithm.hpp>

#include <filesystem>
#include <string>
#include <tuple>
#include <optional>
#include <string_view>

namespace engine
{
	class EntityState;
	class MetaParsingContext;

	struct EntityStateRule;
	struct EntityThreadDescription;
	struct MetaEvaluationContext;

	struct EntityConstructionContext;

	class EntityFactory : protected EntityFactoryContext
	{
		public:
			using FactoryKey = std::string; // std::string_view;
			using SmallSize = MetaTypeDescriptor::SmallSize;

			//using json = util::json;
		protected:
			EntityDescriptor descriptor;

			std::optional<EntityStateIndex> default_state_index = std::nullopt;

		public:
			EntityFactory() = default;

			template <typename ChildFactoryCallback>
			inline EntityFactory
			(
				const EntityFactoryContext& factory_context,
				ChildFactoryCallback&& child_callback,
				const MetaParsingContext& opt_parsing_context={},
				bool resolve_external_modules=true, bool process_children=true
			)
				: EntityFactoryContext(factory_context), descriptor(factory_context)
			{
				process_archetype(descriptor, paths.instance_path, paths.instance_directory, child_callback, opt_parsing_context, this, resolve_external_modules, process_children, &default_state_index);
			}

			inline EntityFactory
			(
				const EntityFactoryContext& factory_context,
				const MetaParsingContext& opt_parsing_context={},
				bool resolve_external_modules=true
			)
				: EntityFactoryContext(factory_context), descriptor(factory_context)
			{
				process_archetype(descriptor, paths.instance_path, paths.instance_directory, opt_parsing_context, this, resolve_external_modules, &default_state_index);
			}

			EntityFactory(const EntityFactory&) = default;
			EntityFactory(EntityFactory&&) noexcept = default;

			EntityFactory& operator=(const EntityFactory&) = default;
			EntityFactory& operator=(EntityFactory&&) noexcept = default;

			// NOTE: Components whose instantiation requires indirection are not instantiated by this routine.
			// (i.e. Components that have dependence on `InstanceComponent` or `EntityDescriptor`)
			// 
			// See also: `EntityFactoryData::on_entity_create`
			Entity create(const EntityConstructionContext& context) const;

			inline Entity operator()(const EntityConstructionContext& context) const
			{
				return create(context);
			}

			inline const EntityDescriptor& get_descriptor() const
			{
				return descriptor;
			}

			inline const std::filesystem::path& get_instance_path() const
			{
				return paths.instance_path;
			}

			inline const std::optional<EntityStateIndex>& get_default_state_index() const
			{
				return default_state_index;
			}
	};
}