#pragma once

#include "types.hpp"
#include "entity_descriptor.hpp"

#include <util/json.hpp>

#include <filesystem>
#include <tuple>
#include <optional>
#include <string_view>

namespace engine
{
	//class World;
	class ResourceManager;

	// TODO: Look into moving `registry` and `resource_manager` out of this type.
	struct EntityFactoryContext
	{
		//EntityFactoryContext(const EntityFactoryContext&) = default;
		//EntityFactoryContext(EntityFactoryContext&&) noexcept = default;

		//EntityFactoryContext& operator=(const EntityFactoryContext&) = default;
		//EntityFactoryContext& operator=(EntityFactoryContext&&) noexcept = default;

		//using ServiceType = World;
		//ServiceType& service;

		Registry& registry;
		ResourceManager& resource_manager;

		struct
		{
			std::filesystem::path instance_path = {};

			// The local root directory for instances created with this factory.
			std::filesystem::path instance_directory = {};

			std::filesystem::path service_archetype_root_path = {};
			std::filesystem::path archetype_root_path = "archetypes"; // "engine/archetypes";
		} paths;
	};

	//template <typename ServiceType>
	struct EntityConstructionContext
	{
		//std::filesystem::path instance_path;

		Entity parent = null;

		// If this field is left as `null`, a factory will
		// generate an appropriate Entity instance.
		Entity entity = null;
	};

	class EntityFactory : protected EntityFactoryContext
	{
		protected:
			EntityDescriptor descriptor;

			std::optional<EntityStateIndex> default_state_index = std::nullopt;
		public:
			using SmallSize = MetaTypeDescriptor::SmallSize;

			//using json = util::json;

			std::filesystem::path resolve_reference(const std::filesystem::path& path, const std::filesystem::path& base_path) const;

			EntityFactory(const EntityFactory&) = default;
			EntityFactory(EntityFactory&&) noexcept = default;

			EntityFactory& operator=(const EntityFactory&) = default;
			EntityFactory& operator=(EntityFactory&&) noexcept = default;

			EntityFactory(const EntityFactoryContext& factory_context);

			Entity create(EntityConstructionContext context={}) const;

			inline Entity operator()(EntityConstructionContext context = {}) const
			{
				return create(context);
			}

			inline const EntityDescriptor& get_descriptor() const
			{
				return descriptor;
			}
		protected:
			static std::tuple<std::string_view, bool, std::optional<SmallSize>>
			parse_component_declaration(const std::string& component_declaration);

			bool process_component
			(
				EntityDescriptor::TypeInfo& components_out,
				std::string_view component_name,
				const util::json* data=nullptr,
				bool allow_inplace_changes=false,
				std::optional<SmallSize> constructor_arg_count=std::nullopt
			);

			bool process_state
			(
				EntityDescriptor::StateCollection& states_out,
				const util::json& data,
				std::string_view state_name
			);

			void process_component_list
			(
				EntityDescriptor::TypeInfo& components_out,
				const util::json& components
			);

			void process_archetype(const util::json& data, const std::filesystem::path& base_path, bool resolve_external_modules=true);

			// Resolves the contents of the `archetypes` field pointed to by `instance`.
			// 
			// If an `archetypes` field is not present in `instance`,
			// or if its contents could not be read, this will return false.
			// 
			// See also: `process_archetype`.
			bool resolve_archetypes(const util::json& instance, const std::filesystem::path& base_path);
	};
}