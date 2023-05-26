#pragma once

#include "entity_listener.hpp"
#include "entity_thread_cadence.hpp"

#include <engine/types.hpp>
#include <engine/basic_system.hpp>

#include <engine/meta/types.hpp>
#include <engine/meta/meta_variable_scope.hpp>

#include <util/small_vector.hpp>

#include <string_view>
#include <optional>
#include <unordered_map>

namespace engine
{
	class Service;

	class EntityDescriptor;

	struct OnServiceUpdate;
	struct OnServiceFixedUpdate;
	struct OnStateChange;
	struct OnStateActivate;

	struct OnComponentCreate;
	struct OnComponentUpdate;
	struct OnComponentDestroy;

	struct OnParentChanged;

	struct StateChangeCommand;
	struct StateActivationCommand;

	struct EntityThreadSpawnCommand;
	struct EntityThreadStopCommand;
	struct EntityThreadPauseCommand;
	struct EntityThreadResumeCommand;
	struct EntityThreadAttachCommand;
	struct EntityThreadDetachCommand;
	struct EntityThreadUnlinkCommand;
	struct EntityThreadSkipCommand;
	struct EntityThreadRewindCommand;

	struct EntityThread;
	struct EntityThreadDescription;
	struct EntityThreadComponent;
	struct EntityInstruction;

	struct RelationshipComponent;
	struct EntityContextComponent;

	class EntitySystem : public BasicSystem
	{
		public:
			// TODO: Separate this function from `EntitySystem`.
			static MetaVariableEvaluationContext resolve_variable_context
			(
				Service* opt_service={},
				Registry* opt_registry={},
				Entity opt_entity=null,

				EntityThreadComponent* opt_thread_comp={},
				EntityThread* opt_thread={},

				EntityContextComponent* opt_context_comp={},

				std::optional<MetaVariableScope> referenced_scope=std::nullopt
			);

			EntitySystem
			(
				Service& service,
				SystemManagerInterface& system_manager,
				bool subscribe_immediately=false
			);

			EntitySystem
			(
				Service& service,
				bool subscribe_immediately=false
			);

			bool set_state(Entity entity, std::string_view state_name) const;
			bool set_state(Entity entity, StringHash state_name) const;
			bool set_state_by_id(Entity entity, StringHash state_id) const;
		protected:
			std::unordered_map<MetaTypeID, EntityListener> listeners;

			SystemManagerInterface* system_manager = nullptr;

			// Used internally by `on_state_activation_command` for 'delayed activation' behavior.
			bool activate_state(Entity entity, StringHash state_id) const;

			Registry& get_registry() const;

			const EntityDescriptor* get_descriptor(Entity entity) const;
			std::optional<EntityStateIndex> get_state(Entity entity) const;

			// Triggered when this system subscribes to a service.
			bool on_subscribe(Service& service) override;
			bool on_unsubscribe(Service& service) override;

			void on_update(const OnServiceUpdate& data);
			void on_fixed_update(const OnServiceFixedUpdate& data);

			void on_state_init(Registry& registry, Entity entity);
			void on_state_update(Registry& registry, Entity entity);
			void on_state_destroyed(Registry& registry, Entity entity);

			void on_state_change(const OnStateChange& state_change);
			void on_state_activate(const OnStateActivate& state_activate);

			void on_state_change_command(const StateChangeCommand& state_change);
			void on_state_activation_command(const StateActivationCommand& state_activation);

			void on_thread_spawn_command(const EntityThreadSpawnCommand& thread_command);
			void on_thread_stop_command(const EntityThreadStopCommand& thread_command);
			void on_thread_pause_command(const EntityThreadPauseCommand& thread_command);
			void on_thread_resume_command(const EntityThreadResumeCommand& thread_command);
			void on_thread_attach_command(const EntityThreadAttachCommand& thread_command);
			void on_thread_detach_command(const EntityThreadDetachCommand& thread_command);
			void on_thread_unlink_command(const EntityThreadUnlinkCommand& thread_command);
			void on_thread_skip_command(const EntityThreadSkipCommand& thread_command);
			void on_thread_rewind_command(const EntityThreadRewindCommand& thread_command);

			void on_component_create(const OnComponentCreate& component_details);
			void on_component_update(const OnComponentUpdate& component_details);
			void on_component_destroy(const OnComponentDestroy& component_details);

			void on_parent_changed(const OnParentChanged& parent_changed);
			
			void on_context_init(Registry& registry, Entity entity);
			void realign_child_contexts(Registry& registry, Entity entity, EntityContextComponent& context_comp, const RelationshipComponent& relationship_comp);

			void on_parent_context_changed(Registry& registry, const OnParentChanged& parent_changed);

			// The return value of this function indicates if the parent's context was used.
			// NOTE: This routine does not mark `context_comp` as patched.
			bool use_parent_context_or_generate(Registry& registry, Entity entity, EntityContextComponent& context_comp, const RelationshipComponent& relationship_comp, bool realign_children=true, bool keep_existing=true);

			EntityInstructionIndex step_thread
			(
				Registry& registry,
				Entity entity,

				const EntityDescriptor& descriptor,
				const EntityThreadDescription& source,

				EntityThreadComponent& thread_comp,
				EntityThread& thread
			);
		private:
			template <EntityThreadCadence... target_cadence>
			std::size_t progress_threads(); // EntityThreadCount

			template <bool allow_emplace, typename EventType=void, typename ThreadCommandType=void, typename RangeCallback=void, typename IDCallback=void, typename ...EventArgs>
			bool thread_command_impl(const ThreadCommandType& thread_command, RangeCallback&& range_callback, IDCallback&& id_callback, std::string_view dbg_name, std::string_view dbg_name_past_tense, EventArgs&&... event_args);

			EntityListener* listen(MetaTypeID event_type_id);

			void on_state_update_impl(Registry& registry, Entity entity, bool handle_existing=true);
	};
}