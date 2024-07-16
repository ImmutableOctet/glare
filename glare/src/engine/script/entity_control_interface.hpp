#pragma once

#include <engine/types.hpp>
#include <engine/transform.hpp>

#include <engine/components/name_component.hpp>
#include <engine/components/player_component.hpp>
#include <engine/components/player_target_component.hpp>

#include <engine/entity/types.hpp>
#include <engine/entity/entity_target.hpp>
#include <engine/entity/entity_thread_flags.hpp>
#include <engine/entity/components/entity_target_component.hpp>

#include <engine/world/delta/delta_system.hpp>

#include <engine/world/motion/components/focus_component.hpp>

#include <type_traits>
#include <utility>

namespace engine
{
	struct PlayerComponent;

	class EntityControlInterface
	{
		public:
			EntityControlInterface() = default;

			EntityControlInterface(const EntityControlInterface&) = delete;
			EntityControlInterface(EntityControlInterface&&) noexcept = default;

			EntityControlInterface& operator=(const EntityControlInterface&) = delete;
			EntityControlInterface& operator=(EntityControlInterface&&) noexcept = default;

			Entity get_entity(this auto&& self, auto&& entity_name)
			{
				return self.get_entity_by_name(std::forward<decltype(entity_name)>(entity_name));
			}

			template <typename ComponentType, typename ...Args>
			decltype(auto) get_or_add_to(this auto&& self, Entity entity, Args&&... args)
			{
				return self.get_registry().get_or_emplace<ComponentType>(entity, std::forward<Args>(args)...);
			}

			template <typename ComponentType, typename ...Args>
			decltype(auto) get_or_add(this auto&& self, Entity entity, Args&&... args)
			{
				return self.get_or_add_to(entity, std::forward<Args>(args)...);
			}

			Entity get(this auto&& self, Entity entity, const EntityTarget& target)
			{
				return self.get_target(entity, target);
			}

			template <typename ComponentType>
			decltype(auto) get(this auto&& self, Entity entity)
			{
				if constexpr (std::is_const_v<std::remove_reference_t<decltype(self)>>)
				{
					return self.get_registry().get<ComponentType>(entity);
				}
				else
				{
					return self.get_registry().patch<ComponentType>(entity);
				}
			}

			template
			<
				typename ComponentType,
				typename PatchFn,

				//std::enable_if<std::is_invocable_v<PatchFn, ComponentType&>, bool>::type=true
				std::enable_if<(!std::is_same_v<std::remove_cvref_t<PatchFn>, Entity>), bool>::type=true
			>
			decltype(auto) get(this auto&& self, Entity entity, PatchFn&& patch_fn)
			{
				return self.template patch<ComponentType>(entity, std::forward<PatchFn>(patch_fn));
			}

			template <typename ComponentType, typename PatchFn>
			decltype(auto) patch(this auto&& self, Entity entity, PatchFn&& patch_fn)
			{
				return self.get_registry().patch<ComponentType>(entity, std::forward<PatchFn>(patch_fn));
			}

			template <typename ComponentType>
			auto try_get(this auto&& self, Entity entity)
			{
				auto& registry = self.get_registry();

				auto component_instance = registry.try_get<ComponentType>(entity);

				if (component_instance)
				{
					// Mark `ComponentType` as patched for `entity`.
					registry.patch<ComponentType>(entity);
				}

				return component_instance;
			}

			template <typename ComponentType, typename PatchFn>
			auto try_get(this auto&& self, Entity entity, PatchFn&& patch_fn)
			{
				auto& registry = self.get_registry();

				if constexpr ((std::is_const_v<std::remove_reference_t<decltype(self)>>) || ((std::is_invocable_v<PatchFn, const ComponentType&>) && (!std::is_invocable_v<PatchFn, ComponentType&>)))
				{
					const auto component_instance = registry.try_get<ComponentType>(entity);

					return component_instance;
				}
				else
				{
					auto component_instance = registry.try_get<ComponentType>(entity);

					if (component_instance)
					{
						// Mark `ComponentType` as patched for `entity`.
						registry.patch<ComponentType>(entity, std::forward<PatchFn>(patch_fn));
					}

					return component_instance;
				}
			}

			Entity get_entity_by_name(this auto&& self, StringHash entity_name)
			{
				return self.get(EntityTarget { EntityTarget::EntityNameTarget { entity_name } });
			}

			Entity get_entity_by_name(this auto&& self, std::string_view entity_name)
			{
				return self.get(EntityTarget { EntityTarget::EntityNameTarget::from_string(entity_name) });
			}

			std::string_view get_name(this auto&& self, Entity entity)
			{
				if (const auto name_component = self.template try_get<NameComponent>(entity))
				{
					return name_component->get_name();
				}

				return {};
			}

			decltype(auto) name(this auto&& self, Entity entity)
			{
				return self.get_name(entity);
			}

			StringHash get_name_hash(this auto&& self, Entity entity)
			{
				if (const auto name_component = self.template try_get<NameComponent>(entity))
				{
					return name_component->hash();
				}

				return {};
			}

			StringHash name_hash(this auto&& self, Entity entity)
			{
				return self.get_name_hash(entity);
			}

			Entity get_parent(this auto&& self, Entity entity)
			{
				return self.get(EntityTarget { EntityTarget::ParentTarget {} });
			}

			Entity parent(this auto&& self, Entity entity)
			{
				return self.get_parent(entity);
			}

			Entity get_child(this auto&& self, Entity entity, StringHash child_name)
			{
				return self.get(entity, EntityTarget { EntityTarget::ChildTarget { child_name } });
			}

			Entity get_child(this auto&& self, Entity entity, std::string_view child_name)
			{
				return get(entity, EntityTarget { EntityTarget::ChildTarget::from_string(child_name) });
			}

			Entity child(this auto&& self, Entity entity, auto&& child_name)
			{
				return self.get_child(entity, std::forward<decltype(child_name)>(child_name));
			}

			Transform get_transform(this auto&& self, Entity entity)
			{
				return Transform { self.get_registry(), entity };
			}

			decltype(auto) transform(this auto&& self, Entity entity)
			{
				return self.get_transform(entity);
			}

			decltype(auto) get_delta_system(this auto&& self)
			{
				return self.template system<DeltaSystem>();
			}

			float get_delta(this auto&& self)
			{
				// TODO: Implement delta value lookup based on LOD batching,
				// then use value from delta-system as fallback.

				if (const auto delta_system = self.template try_get_system<DeltaSystem>())
				{
					return delta_system->get_delta();
				}

				return 1.0f;
			}

			bool is_player(this auto&& self, Entity entity)
			{
				return static_cast<bool>(self.template try_get<PlayerComponent>(entity));
			}

			Entity get_player(this auto&& self, PlayerIndex player_index)
			{
				return self.get(null, EntityTarget { EntityTarget::PlayerTarget { player_index } });
			}

			Entity get_player(this auto&& self, Entity player_or_targeting_player)
			{
				if (const auto as_player = self.get_as_player(player_or_targeting_player); as_player != null)
				{
					return as_player;
				}

				return self.get_target_player(player_or_targeting_player);
			}

			Entity player(this auto&& self, PlayerIndex player_index)
			{
				return self.get_player(player_index);
			}

			Entity player(this auto&& self, Entity player_or_targeting_player)
			{
				return self.get_player(player_or_targeting_player);
			}

			Entity get_as_player(this auto&& self, Entity entity)
			{
				if (const auto player_component = self.template try_get<PlayerComponent>(entity))
				{
					return self.get_player(player_component->player_index);
				}

				return null;
			}

			Entity get_target_player(this auto&& self, Entity entity)
			{
				if (const auto player_target_component = self.template try_get<PlayerTargetComponent>(entity))
				{
					return self.get_player(player_target_component->player_index);
				}

				return null;
			}

			PlayerIndex get_index_from_player(this auto&& self, Entity player)
			{
				if (const auto player_component = self.template try_get<PlayerComponent>(player))
				{
					return player_component->player_index;
				}

				return NO_PLAYER;
			}

			PlayerIndex get_player_index(this auto&& self, Entity entity)
			{
				if (const auto player_index = self.get_index_from_player(entity); player_index != NO_PLAYER)
				{
					return player_index;
				}

				if (const auto player_index = self.get_target_player_index(entity); player_index != NO_PLAYER)
				{
					return player_index;
				}

				return NO_PLAYER;
			}

			PlayerIndex get_target_player_index(this auto&& self, Entity entity)
			{
				if (const auto player_target_component = self.try_get<PlayerTargetComponent>(entity))
				{
					return player_target_component->player_index;
				}

				return NO_PLAYER;
			}

			Entity get_target(this auto&& self, Entity entity, const EntityTarget& target)
			{
				return target.get(self.get_registry(), entity, self.get_context());
			}

			Entity get_target(this auto&& self, Entity entity)
			{
				if (const auto target_component = self.try_get<EntityTargetComponent>())
				{
					if (const auto target_entity = self.get_target(entity, static_cast<EntityTarget>(*target_component)); target_entity != null)
					{
						return target_entity;
					}
				}

				if (const auto focus_component = self.try_get<FocusComponent>())
				{
					if (focus_component->target != null)
					{
						return focus_component->target;
					}
				}

				if (const auto player_target = self.get_target_player(entity); player_target != null)
				{
					return player_target;
				}

				return null;
			}

			Entity target(this auto&& self, Entity entity, const EntityTarget& target)
			{
				return self.get_target(entity, target);
			}

			Entity target(this auto&& self, Entity entity)
			{
				return self.get_target(entity);
			}

			decltype(auto) start_thread(this auto&& self, auto&& script_entry_point, Entity entity, auto&&... additional_args)
			{
				return self.start_thread_impl
				(
					std::forward<decltype(script_entry_point)>(script_entry_point),
					entity, entity,
					std::forward<decltype(additional_args)>(additional_args)...
				);
			}

			decltype(auto) start_script(this auto&& self, auto&& script_entry_point, Entity entity, auto&&... additional_args)
			{
				return self.start_script_impl
				(
					std::forward<decltype(script_entry_point)>(script_entry_point),
					entity, entity,
					
					std::forward<decltype(additional_args)>(additional_args)...
				);
			}
	};
}