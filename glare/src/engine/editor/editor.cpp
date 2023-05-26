#include "editor.hpp"

#include <engine/service.hpp>
#include <engine/config.hpp>
#include <engine/resource_manager/resource_manager.hpp>
#include <engine/entity/entity_factory_context.hpp>
#include <engine/entity/entity_construction_context.hpp>

namespace engine
{
	Editor::Editor
	(
		Service& service,
		const Config& config,
		ResourceManager& resource_manager,
		bool subscribe_immediately
	) :
		BasicSystem(service),
		resource_manager(resource_manager)
	{
		auto& registry = service.get_registry();

		instance = resource_manager.generate_entity
		(
			{
				{
					.instance_path               = "engine/editor/editor.json",
					.instance_directory          = "engine/editor",
					.shared_directory            = (std::filesystem::path(config.entity.archetype_path) / "editor"),
					.service_archetype_root_path = (std::filesystem::path(config.entity.archetype_path) / "world"),
					.archetype_root_path         = config.entity.archetype_path
				}
			},

			{
				//service,

				.registry = registry,
				.resource_manager = resource_manager,

				.parent = service.get_root()
			}
		);

		assert(instance != null);

		if (subscribe_immediately)
		{
			subscribe();
		}
	}

	Editor::~Editor() {}

	bool Editor::on_subscribe(Service& service)
	{
		return true;
	}

	Entity Editor::get_root() const
	{
		return instance;
	}
}