#pragma once

#include <engine/types.hpp>
#include <engine/basic_system.hpp>
#include <engine/meta/types.hpp>

#include <string_view>
#include <optional>

namespace engine
{
	class Service;
	class ResourceManager;
	struct OnServiceUpdate;
	struct EntityDescriptor;

	class StateSystem : public BasicSystem
	{
		public:
			StateSystem(Service& service, const ResourceManager& resource_manager, bool subscribe_immediately=false);

			bool set_state(Entity entity, std::string_view state_name) const;
			bool set_state_by_id(Entity entity, StringHash state_id) const;
			bool set_state_by_index(Entity entity, EntityStateIndex state_index) const;
		protected:
			Registry& get_registry() const;

			const EntityDescriptor* get_descriptor(Entity entity) const;
			std::optional<EntityStateIndex> get_state(Entity entity) const;

			// Triggered when this system subscribes to a service.
			bool on_subscribe(Service& service) override;

			// Triggered once per service update; used to handle
			// things like 'state changed' (`OnInput`) events, etc.
			void on_update(const OnServiceUpdate& data);

			const ResourceManager& resource_manager;
	};
}