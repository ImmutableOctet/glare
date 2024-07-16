#pragma once

#include <engine/types.hpp>

#include <engine/entity/entity_target.hpp>

#include <type_traits>
#include <utility>

namespace engine
{
	class WorldSystem;

	namespace impl
	{
		template <typename T>
		inline constexpr bool is_system_type = std::is_base_of_v<WorldSystem, T>;
	}

	class EntitySelfInterface
	{
		public:
			EntitySelfInterface() = default;

			EntitySelfInterface(Entity entity) :
				_entity(entity)
			{}

			EntitySelfInterface(const EntitySelfInterface&) = delete;
			EntitySelfInterface(EntitySelfInterface&&) noexcept = default;

			EntitySelfInterface& operator=(const EntitySelfInterface&) = delete;
			EntitySelfInterface& operator=(EntitySelfInterface&&) noexcept = default;

			inline Entity get_entity() const
			{
				return _entity;
			}

			template <typename ComponentType, typename ...Args>
			decltype(auto) get_or_add(this auto&& self, Args&&... args)
			{
				return self.template get_or_add_to<ComponentType>(self.get_entity(), std::forward<Args>(args)...);
			}

			Entity get(this auto&& self, const EntityTarget& target)
			{
				return self.get_target(target);
			}
			
			template <typename ComponentOrSystemType>
			decltype(auto) get(this auto&& self)
			{
				if constexpr (impl::is_system_type<ComponentOrSystemType>)
				{
					return self.template system<ComponentOrSystemType>();
				}
				else
				{
					return self.template get<ComponentOrSystemType>(self.get_entity());
				}
			}

			template
			<
				typename ComponentType,
				typename PatchFn,
				
				std::enable_if<(!std::is_same_v<std::remove_cvref_t<PatchFn>, Entity>), bool>::type=true
			>
			decltype(auto) get(this auto&& self, PatchFn&& patch_fn)
			{
				return self.template get<ComponentType>(self.get_entity(), std::forward<PatchFn>(patch_fn));
			}

			template <typename ComponentType, typename PatchFn>
			decltype(auto) patch(this auto&& self, PatchFn&& patch_fn)
			{
				return self.template patch<ComponentType>(self.get_entity(), std::forward<PatchFn>(patch_fn));
			}

			template <typename ComponentOrSystemType>
			auto try_get(this auto&& self)
			{
				if constexpr (impl::is_system_type<ComponentOrSystemType>)
				{
					return self.template try_get_system<ComponentOrSystemType>();
				}
				else
				{
					return self.template try_get<ComponentOrSystemType>(self.get_entity());
				}
			}

			inline Entity this_entity() const
			{
				return get_entity();
			}

			inline Entity self_entity() const
			{
				return get_entity();
			}

			decltype(auto) get_name(this auto&& self)
			{
				return self.get_name(self.get_entity());
			}

			decltype(auto) get_name_hash(this auto&& self)
			{
				return self.get_name_hash(self.get_entity());
			}

			decltype(auto) name(this auto&& self)
			{
				return self.get_name();
			}

			decltype(auto) name_hash(this auto&& self)
			{
				return self.name_hash(self.get_entity());
			}

			Entity get_parent(this auto&& self)
			{
				return self.get_parent(self.get_entity());
			}

			Entity parent(this auto&& self)
			{
				return self.parent(self.get_entity());
			}

			Entity get_child(this auto&& self, StringHash child_name)
			{
				return self.get(self.get_entity(), child_name);
			}

			Entity get_child(this auto&& self, std::string_view child_name)
			{
				return self.get(self.get_entity(), child_name);
			}

			Entity child(this auto&& self, auto&& child_name)
			{
				return self.child(self.get_entity(), std::forward<decltype(child_name)>(child_name));
			}

			Transform get_transform(this auto&& self)
			{
				return self.get_transform(self.get_entity());
			}

			decltype(auto) transform(this auto&& self)
			{
				return self.transform(self.get_entity());
			}

			bool is_player(this auto&& self)
			{
				return self.is_player(self.get_entity());
			}

			Entity get_player(this auto&& self, PlayerIndex player_index)
			{
				return self.get(EntityTarget { EntityTarget::PlayerTarget { player_index } });
			}

			Entity get_player(this auto&& self)
			{
				return self.get_player(self.get_entity());
			}

			Entity player(this auto&& self)
			{
				return self.get_player();
			}

			Entity get_as_player(this auto&& self)
			{
				return self.get_as_player(self.get_entity());
			}

			Entity get_target_player(this auto&& self)
			{
				return self.get_target_player(self.get_entity());
			}

			PlayerIndex get_player_index(this auto&& self)
			{
				return self.get_player_index(self.get_entity());
			}

			PlayerIndex get_target_player_index(this auto&& self)
			{
				return self.get_target_player_index(self.get_entity());
			}

			Entity get_target(this auto&& self)
			{
				return self.get_target(self.get_entity());
			}

			Entity get_target(this auto&& self, const EntityTarget& target)
			{
				return self.get_target(self.get_entity(), target);
			}

			Entity target(this auto&& self, const EntityTarget& target)
			{
				return self.target(self.get_entity(), target);
			}

			Entity target(this auto&& self)
			{
				return self.target(self.get_entity());
			}

			decltype(auto) start_thread(this auto&& self, auto&& script_entry_point, Entity entity)
			{
				return self.start_thread_impl
				(
					std::forward<decltype(script_entry_point)>(script_entry_point),
					self.get_entity(), entity
				);
			}

			decltype(auto) start_script(this auto&& self, auto&& script_entry_point, Entity entity)
			{
				return self.start_script_impl
				(
					std::forward<decltype(script_entry_point)>(script_entry_point),
					self.get_entity(), entity
				);
			}

			decltype(auto) start_thread(this auto&& self, auto&& thread_entry_point, auto&&... additional_args)
			{
				return self.start_thread_impl
				(
					std::forward<decltype(thread_entry_point)>(thread_entry_point),
					self.get_entity(), self.get_entity(),
					std::forward<decltype(additional_args)>(additional_args)...
				);
			}

			decltype(auto) start_script(this auto&& self, auto&& script_entry_point, auto&&... additional_args)
			{
				const auto self_entity = self.get_entity();
				const auto& target_entity = self_entity;

				return self.start_script_impl
				(
					std::forward<decltype(script_entry_point)>(script_entry_point),

					self_entity, target_entity,
					
					std::forward<decltype(additional_args)>(additional_args)...
				);
			}

			template <typename ScriptType>
			decltype(auto) start_script(this auto&& self, auto&&... additional_args)
			{
				const auto self_entity = self.get_entity();
				const auto& target_entity = self_entity;

				return self.start_script_impl
				(
					ScriptType { self.get_registry(), target_entity, self.get_context() },

					self_entity, target_entity,

					std::forward<decltype(additional_args)>(additional_args)...
				);
			}

			inline explicit operator Entity() const
			{
				return get_entity();
			}

			inline explicit operator bool() const
			{
				return ((_entity != null));
			}

		protected:
			inline void set_entity(Entity entity)
			{
				_entity = entity;
			}

			Entity _entity = null;
	};
}