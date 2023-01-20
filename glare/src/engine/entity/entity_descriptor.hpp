#pragma once

#include "types.hpp"
#include "entity_state.hpp"
#include "entity_thread_description.hpp"
#include "entity_thread_range.hpp"

#include <engine/meta/meta_description.hpp>
#include <engine/meta/meta_type_descriptor.hpp>

#include <optional>
#include <memory>
#include <utility>
#include <type_traits>
//#include <cstddef>

#include <util/shared_storage.hpp>

namespace engine
{
	class EntityDescriptor
	{
		public:
			using TypeInfo = MetaDescription;

			// Could also be handled as a map-type, but that would
			// be less efficient for what is normally a small number of states.
			using StateCollection = util::small_vector<std::unique_ptr<EntityState>, 16>; // util::small_vector<EntityState, 4>; // std::shared_ptr<...>;

			using ThreadCount = EntityThreadCount; // std::uint16_t; // std::uint8_t
			using ImmediateThreadDetails = EntityThreadRange;

			// Statically assigned components.
			MetaDescription components;

			// Dynamic component permutations, applied as state changes.
			StateCollection states;

			util::small_vector<ImmediateThreadDetails, 1> immediate_threads; // 2

			using SharedStorage = util::SharedStorage
			<
				MetaTypeDescriptor,
				EventTriggerCondition,
				EntityThreadDescription
			>;

			SharedStorage shared_storage;

			// TODO: Optimize/bette integrate with `SharedStorage`.
			template <typename ResourceType, typename ...Args>
			EntityDescriptorShared<ResourceType> allocate(Args&&... args)
			{
				auto& allocated_resource = shared_storage.allocate<ResourceType>(std::forward<Args>(args)...);

				return EntityDescriptorShared<ResourceType> { *this, allocated_resource };
			}

			template <typename ResourceType>
			EntityDescriptorShared<ResourceType> allocate(ResourceType&& resource)
			{
				auto allocated_resource_idx = shared_storage.allocate<ResourceType>(std::forward<ResourceType>(resource));

				return EntityDescriptorShared<ResourceType> { allocated_resource_idx };
			}

			MetaTypeDescriptor& generate_empty_command(const MetaType& command_type, Entity source=null, Entity target=null);

			inline SharedStorage& get_shared_storage() { return shared_storage; }
			inline const SharedStorage& get_shared_storage() const { return shared_storage; }

			inline const auto& get_threads() const
			{
				return shared_storage.data<EntityThreadDescription>();
			}

			// TODO: Optimize via map lookup, etc.
			const EntityThreadDescription& get_thread(EntityThreadIndex thread_index) const;

			inline EntityThreadIndex get_next_thread_index() const
			{
				return shared_storage.get_next_index<EntityThreadDescription>();
			}

			std::optional<EntityThreadIndex> get_thread_index(EntityThreadID thread_id) const;

			inline std::optional<EntityThreadIndex> get_thread_index(std::optional<EntityThreadID> thread_id) const
			{
				if (!thread_id.has_value())
				{
					return std::nullopt;
				}

				return get_thread_index(*thread_id);
			}

			std::optional<EntityThreadID> get_thread_id(EntityThreadIndex thread_index) const;

			const EntityState* get_state(StringHash name) const;
			EntityState* get_state(StringHash name);

			const EntityState* get_state(std::optional<StringHash> name) const;
			EntityState* get_state(std::optional<StringHash> name);

			std::optional<EntityStateIndex> get_state_index(EntityStateID name) const;

			inline std::optional<EntityStateIndex> get_state_index(std::optional<EntityStateID> name) const
			{
				if (!name)
				{
					return std::nullopt;
				}

				return get_state_index(*name);
			}

			const EntityState* get_state_by_index(EntityStateIndex state_index) const;

			bool set_state(Registry& registry, Entity entity, std::optional<EntityStateIndex> previous_state, std::string_view state_name) const;
			bool set_state_by_id(Registry& registry, Entity entity, std::optional<EntityStateIndex> previous_state, StringHash state_id) const;
			bool set_state_by_index(Registry& registry, Entity entity, std::optional<EntityStateIndex> previous_state, EntityStateIndex state_index) const;
	};
}