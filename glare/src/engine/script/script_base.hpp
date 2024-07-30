#pragma once

#include "script_interface.hpp"
#include "script_bootstrap_impl.hpp"
#include "script_input_interface.hpp"
#include "contextual_interface.hpp"
#include "registry_control_interface.hpp"
#include "entity_control_interface.hpp"
#include "entity_self_interface.hpp"
#include "entity_thread_interface.hpp"
#include "entity_state_self_interface.hpp"

#include <engine/types.hpp>

namespace engine
{
	class World;

	class EntitySystem;
	class EntityState;

	class DeltaSystem;

	class ScriptBase :
		public ScriptInterface,
		public ScriptBootstrapImpl,
		public ScriptInputInterface,
		public ContextualInterface,
		public RegistryControlInterface,
		public EntityControlInterface,
		public EntitySelfInterface,
		public EntityThreadInterface,
		public EntityStateSelfInterface
	{
		public:
			using ScriptID = StringHash;

			ScriptBase() = default;

			inline ScriptBase
			(
				const MetaEvaluationContext& context,
				Registry& registry,
				ScriptID script_id,
				Entity entity
			) :
				ContextualInterface(context),
				RegistryControlInterface(registry),
				EntitySelfInterface(entity),

				_script_id(script_id)
			{}

			ScriptBase(const ScriptBase&) = delete;
			ScriptBase(ScriptBase&&) noexcept = default;

			ScriptBase& operator=(const ScriptBase&) = delete;
			ScriptBase& operator=(ScriptBase&&) noexcept = default;

			inline ScriptID get_script_id() const
			{
				return _script_id;
			}

			inline bool has_script_id() const
			{
				return static_cast<bool>(_script_id);
			}

			inline explicit operator Entity() const
			{
				return EntitySelfInterface::operator Entity();
			}

			inline explicit operator bool() const
			{
				return
				(
					(ContextualInterface::operator bool())
					&&
					(RegistryControlInterface::operator bool())
					&&
					(EntitySelfInterface::operator bool())
					&&
					(has_script_id())
				);
			}

			inline const MetaEvaluationContext& get_context() const
			{
				return ContextualInterface::get_context();
			}
			
			inline Registry& get_registry() const
			{
				return RegistryControlInterface::get_registry();
			}

			inline Entity get_entity() const
			{
				return EntitySelfInterface::get_entity();
			}

			inline [[nodiscard]] const Service& get_service() const
			{
				return ContextualInterface::get_service();
			}

			inline [[nodiscard]] Service& get_service()
			{
				return ContextualInterface::get_service();
			}

			using ScriptInterface::operator();

			using EntityThreadInterface::event;

			using EntitySelfInterface::get_or_add;

			using EntityControlInterface::get;
			using    EntitySelfInterface::get;

			using EntityControlInterface::try_get;
			using    EntitySelfInterface::try_get;

			using EntityControlInterface::patch;
			using    EntitySelfInterface::patch;

			inline Entity get_player(PlayerIndex player_index) const
			{
				return EntitySelfInterface::get_player(player_index);
			}

			inline Entity get_player(Entity player_or_targeting_player) const
			{
				return EntityControlInterface::get_player(player_or_targeting_player);
			}

			inline Entity get_player() const
			{
				return EntitySelfInterface::get_player();
			}

			using EntityControlInterface::player;
			using    EntitySelfInterface::player;

			using EntityControlInterface::get_target_player;
			using    EntitySelfInterface::get_target_player;

			using EntityControlInterface::get_name;
			using    EntitySelfInterface::get_name;

			using EntityControlInterface::get_name_hash;
			using    EntitySelfInterface::get_name_hash;

			using EntityControlInterface::name;
			using    EntitySelfInterface::name;

			using EntityControlInterface::name_hash;
			using    EntitySelfInterface::name_hash;

			using EntityControlInterface::get_parent;
			using    EntitySelfInterface::get_parent;

			using EntityControlInterface::parent;
			using    EntitySelfInterface::parent;

			using EntityControlInterface::get_child;
			using    EntitySelfInterface::get_child;

			using EntityControlInterface::child;
			using    EntitySelfInterface::child;

			using EntityControlInterface::get_transform;
			using    EntitySelfInterface::get_transform;

			using EntityControlInterface::transform;
			using    EntitySelfInterface::transform;

			using EntityControlInterface::is_player;
			using    EntitySelfInterface::is_player;

			using EntityControlInterface::get_as_player;
			using    EntitySelfInterface::get_as_player;

			using EntityControlInterface::get_player_index;
			using    EntitySelfInterface::get_player_index;

			using EntityControlInterface::get_target_player_index;
			using    EntitySelfInterface::get_target_player_index;

			using EntityControlInterface::get_target;
			using    EntitySelfInterface::get_target;

			using EntityControlInterface::target;
			using    EntitySelfInterface::target;

			using    EntitySelfInterface::start_thread;

			using    EntitySelfInterface::start_script;

		protected:
			// A unique identifier representing this script. (Usually based on the script's path)
			ScriptID _script_id = {};
	};
}