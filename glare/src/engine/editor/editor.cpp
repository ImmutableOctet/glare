#include "editor.hpp"

#include "components/editor_properties_component.hpp"

#include <engine/service.hpp>
#include <engine/config.hpp>

#include <engine/resource_manager/resource_manager.hpp>

#include <engine/entity/entity_factory_context.hpp>
#include <engine/entity/entity_construction_context.hpp>

#include <engine/meta/meta_type_descriptor.hpp>
#include <engine/meta/data_member.hpp>
#include <engine/meta/component.hpp>
#include <engine/meta/hash.hpp>

#include <engine/world/controls/controls.hpp>

#include <util/imgui.hpp>

// MOVE THESE INCLUDES:
#include <string>
#include <string_view>

// Debugging related:
#include <util/log.hpp>

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

	void Editor::on_render()
	{
		auto& service = get_service();
		auto& registry = get_registry();

		ImGui::Begin("Properties");

		registry.view<EditorPropertiesComponent>().each
		(
			[&](Entity entity, EditorPropertiesComponent& properties_comp)
			{
				if (!properties_comp.is_selected)
				{
					return;
				}

				const auto entity_name = service.label(entity);

				if (entity_name.empty())
				{
					return;
				}

				if (ImGui::TreeNode(entity_name.data()))
				{
					if (ImGui::TreeNode("Components"))
					{
						display::components(registry, entity, true);

						ImGui::TreePop();
						ImGui::Spacing();
					}

					ImGui::TreePop();
					ImGui::Spacing();
				}
			}
		);

		ImGui::End();
	}

	Entity Editor::get_root() const
	{
		return instance;
	}
}