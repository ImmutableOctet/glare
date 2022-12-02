#pragma once

#include "entity_rule_listener.hpp"

#include <engine/types.hpp>
#include <engine/basic_system.hpp>
#include <engine/meta/types.hpp>

#include <util/small_vector.hpp>

#include <string_view>
#include <optional>

#include <unordered_map>

namespace engine
{
	class Service;
	class ResourceManager;

	struct EntityDescriptor;

	struct OnServiceUpdate;
	struct OnStateChange;
	struct OnStateActivate;
	struct StateChangeCommand;
	struct StateActivationCommand;

	class StateSystem : public BasicSystem
	{
		public:
			StateSystem(Service& service, const ResourceManager& resource_manager, bool subscribe_immediately=false);

			bool set_state(Entity entity, std::string_view state_name) const;
			bool set_state(Entity entity, StringHash state_name) const;
			bool set_state_by_id(Entity entity, StringHash state_id) const;
		protected:
			// Used internally by `on_state_activation_command` for 'delayed activation' behavior.
			bool activate_state(Entity entity, StringHash state_id) const;

			const ResourceManager& resource_manager;

			std::unordered_map<MetaTypeID, EntityRuleListener> rule_listeners;

			Registry& get_registry() const;

			const EntityDescriptor* get_descriptor(Entity entity) const;
			std::optional<EntityStateIndex> get_state(Entity entity) const;

			// Triggered when this system subscribes to a service.
			bool on_subscribe(Service& service) override;
			bool on_unsubscribe(Service& service) override;

			// Triggered once per service update; used to handle
			// things like 'state changed' (`OnInput`) events, etc.
			void on_update(const OnServiceUpdate& data);

			void on_state_init(Registry& registry, Entity entity);
			void on_state_update(Registry& registry, Entity entity);
			void on_state_destroyed(Registry& registry, Entity entity);

			void on_state_change(const OnStateChange& state_change);
			void on_state_activate(const OnStateActivate& state_activate);

			void on_state_change_command(const StateChangeCommand& state_change);
			void on_state_activation_command(const StateActivationCommand& state_activation);
		private:
			void on_state_update_impl(Registry& registry, Entity entity, bool handle_existing=true);
	};
}