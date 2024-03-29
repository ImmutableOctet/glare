#pragma once

#include "types.hpp"

#include "system_manager_interface.hpp"

#include "meta/meta_evaluation_context.hpp"
#include "entity/entity_instruction.hpp"

#include <cassert>

namespace engine
{
	struct MetaEvaluationContext;

	class Service;
	class World;

	class EntitySystem;
	class EntityState;
	class DeltaSystem;

	// Shared API for C++ script functionality.
	class Script
	{
		public:
			using ScriptID = StringHash;

			Script() = default;

			Script(const Script&) = delete;
			Script(Script&&) noexcept = default;

			Script& operator=(const Script&) = delete;
			Script& operator=(Script&&) noexcept = default;

			Script
			(
				const MetaEvaluationContext& context,
				Registry& registry,
				ScriptID script_id,
				Entity entity
			);

			const MetaEvaluationContext& get_context() const
			{
				assert(_context);

				return *_context;
			}

			const Registry& get_registry() const
			{
				assert(_registry);

				return *_registry;
			}

			Registry& get_registry()
			{
				assert(_registry);

				return *_registry;
			}

			ScriptID get_script_id() const
			{
				return _script_id;
			}

			Entity get_entity() const
			{
				return _entity;
			}

			const Service& get_service() const
			{
				auto& context = get_context();

				assert(context.service);

				return *context.service;
			}

			Service& get_service()
			{
				auto& context = get_context();

				assert(context.service);

				return *context.service;
			}

			SystemManagerInterface& get_system_manager()
			{
				auto& context = get_context();

				assert(context.system_manager);

				return *context.system_manager;
			}

			const SystemManagerInterface& get_system_manager() const
			{
				auto& context = get_context();

				assert(context.system_manager);

				return *context.system_manager;
			}

			template <typename SystemType>
			SystemType* try_get_system()
			{
				auto& system_manager = get_system_manager();

				return system_manager.get_system<SystemType>();
			}

			template <typename SystemType>
			const SystemType* try_get_system() const
			{
				auto& system_manager = get_system_manager();

				return system_manager.get_system<SystemType>();
			}

			template <typename SystemType>
			SystemType& system()
			{
				auto system_instance = try_get_system<SystemType>();

				assert(system_instance);

				return *system_instance;
			}

			template <typename SystemType>
			const SystemType& system() const
			{
				auto system_instance = try_get_system<SystemType>();

				assert(system_instance);

				return *system_instance;
			}

			template <typename ComponentType>
			ComponentType& get(Entity entity)
			{
				// Alternative implementation:
				//return get_registry().get<ComponentType>(entity);

				return get_registry().patch<ComponentType>(entity);
			}

			template <typename ComponentType>
			const ComponentType& get(Entity entity) const
			{
				return get_registry().get<ComponentType>(entity);
			}

			template <typename ComponentType>
			ComponentType& get()
			{
				return get<ComponentType>(get_entity());
			}

			template <typename ComponentType>
			const ComponentType& get() const
			{
				return get<ComponentType>(get_entity());
			}

			template <typename ComponentType>
			ComponentType* try_get(Entity entity)
			{
				auto& registry = get_registry();

				auto component_instance = registry.try_get<ComponentType>(entity);

				if (component_instance)
				{
					// Mark `ComponentType` as patched for `entity`.
					registry.patch<ComponentType>(entity);
				}

				return component_instance;
			}

			template <typename ComponentType>
			const ComponentType* try_get(Entity entity) const
			{
				return get_registry().try_get<ComponentType>(entity);
			}

			template <typename ComponentType>
			ComponentType* try_get()
			{
				return try_get<ComponentType>(get_entity());
			}

			template <typename ComponentType>
			const ComponentType* try_get() const
			{
				return try_get<ComponentType>(get_entity());
			}

			template <typename InstructionType, typename ...InstructionArgs>
			[[nodiscard]] InstructionType instruct(const EntityTarget& target, InstructionArgs&&... instruction_args)
			{
				return InstructionType { target, std::nullopt, std::forward<InstructionArgs>(instruction_args)... };
			}

			template <typename InstructionType, typename ...InstructionArgs>
			[[nodiscard]] InstructionType instruct(const EntityTarget& target, EntityThreadID thread_id, InstructionArgs&&... instruction_args)
			{
				return InstructionType { target, { thread_id }, std::forward<InstructionArgs>(instruction_args)... };
			}

			template <typename InstructionType, typename ...InstructionArgs>
			[[nodiscard]] InstructionType instruct(InstructionArgs&&... instruction_args)
			{
				return InstructionType { EntityTarget {}, std::nullopt, std::forward<InstructionArgs>(instruction_args)... };
			}

			[[nodiscard]] decltype(auto) sleep(auto&&... args) { return instruct<instructions::Sleep>(std::forward<decltype(args)>(args)...); }

			operator Entity() const
			{
				return get_entity();
			}

			explicit operator bool() const
			{
				return
				(
					(_context)
					&&
					(_registry)
					&&
					(_script_id)
					&&
					(_entity != null)
				);
			}

			virtual bool on_update();

			const EntityState* get_state_description() const;
			const EntityState* get_prev_state_description() const;

			EntityStateID get_state_id() const;
			EntityStateID get_prev_state_id() const;

			EntityStateID get_state() const
			{
				return get_state_id();
			}

			EntityStateID get_prev_state() const
			{
				return get_prev_state_id();
			}

			World& get_world();
			const World& get_world() const;

			DeltaSystem& get_delta_system();
			const DeltaSystem& get_delta_system() const;

			float get_delta() const;

		private:
			void update_state_references();
			void update_deltatime();

			const MetaEvaluationContext* _context   = {};
			Registry*                    _registry  = {};
			ScriptID                     _script_id = {};
			Entity                       _entity    = null;

		protected:
			EntityStateID state                     = {};
			EntityStateID prev_state                = {};

			float                        delta      = 1.0f;
	};
}