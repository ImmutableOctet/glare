#pragma once

#include "types.hpp"
#include "entity_state.hpp"
#include "entity_state_collection.hpp"
#include "entity_thread_description.hpp"
#include "entity_thread_range.hpp"
#include "entity_shared_storage.hpp"
#include "meta_description.hpp"

#include <engine/meta/meta_type_descriptor.hpp>

#include <engine/world/physics/collision_config.hpp>

#include <engine/world/animation/animation_repository.hpp>

#include <math/types.hpp>

#include <optional>
#include <memory>
#include <utility>
#include <type_traits>
#include <string_view>
//#include <cstddef>

namespace engine
{
	struct EntityFactoryContext;

	class EntityDescriptor
	{
		public:
			using TypeInfo = MetaDescription;

			using StateCollection = EntityStateCollection;

			using ThreadCount = EntityThreadCount; // std::uint16_t; // std::uint8_t
			using ImmediateThreadDetails = EntityThreadRange;

			using SharedStorage = EntitySharedStorage;

			// TODO: Look into separating this from the `EntityDescriptor` type.
			struct ModelDetails
			{
				using Path = std::string; // std::filesystem::path;

				Path path;

				std::optional<math::Vector> offset = {};

				std::optional<CollisionConfig> collision_cfg;

				bool allow_multiple = true;

				inline explicit operator bool() const
				{
					return !path.empty();
				}
			};

			EntityDescriptor() = default;

			EntityDescriptor(std::string_view path);
			EntityDescriptor(const EntityFactoryContext& factory_context);

			EntityDescriptor(const EntityDescriptor&) = delete;
			EntityDescriptor(EntityDescriptor&&) noexcept = default;

			EntityDescriptor& operator=(const EntityDescriptor&) = delete;
			EntityDescriptor& operator=(EntityDescriptor&&) noexcept = default; // delete;

			// Statically assigned components.
			MetaDescription components;

			// Dynamic component permutations, applied as state changes.
			StateCollection states;

			util::small_vector<ImmediateThreadDetails, 1> immediate_threads; // 2

			ModelDetails model_details;

			AnimationRepository animations;

			SharedStorage shared_storage;

			// TODO: Optimize/better integrate with `SharedStorage`.
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

			const EntityThreadDescription* get_thread_by_id(EntityThreadID thread_id) const;

			const EntityThreadDescription* get_thread_by_id(std::optional<EntityThreadID> thread_id) const
			{
				if (!thread_id)
				{
					return {};
				}

				return get_thread_by_id(*thread_id);
			}

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

			inline bool has_thread(EntityThreadID thread_id) const
			{
				return get_thread_index(thread_id).has_value();
			}

			inline bool has_thread(std::optional<EntityThreadID> thread_id) const
			{
				if (!thread_id.has_value())
				{
					return false;
				}

				return has_thread(*thread_id);
			}

			std::optional<EntityThreadID> get_thread_id(EntityThreadIndex thread_index) const;

			inline const EntityState* get_state(EntityStateID name) const
			{
				return states.get_state(*this, name);
			}

			inline EntityState* get_state(EntityStateID name)
			{
				return states.get_state(*this, name);
			}

			inline const EntityState* get_state(std::optional<EntityStateID> name) const
			{
				return states.get_state(*this, name);
			}

			inline EntityState* get_state(std::optional<EntityStateID> name)
			{
				return states.get_state(*this, name);
			}

			inline std::optional<EntityStateIndex> get_state_index(EntityStateID name) const
			{
				return states.get_state_index(*this, name);
			}

			inline std::optional<EntityStateIndex> get_state_index(std::optional<EntityStateID> name) const
			{
				return states.get_state_index(*this, name);
			}

			inline const EntityState* get_state_by_index(EntityStateIndex state_index) const
			{
				return states.get_state_by_index(*this, state_index);
			}

			inline std::optional<EntityStateID> get_state_name(EntityStateIndex state_index) const
			{
				if (const auto state = get_state_by_index(state_index))
				{
					return state->name;
				}

				return std::nullopt;
			}

			bool set_state(Registry& registry, Entity entity, std::optional<EntityStateIndex> previous_state, std::string_view state_name) const;
			bool set_state_by_id(Registry& registry, Entity entity, std::optional<EntityStateIndex> previous_state, EntityStateID state_id) const;
			bool set_state_by_index(Registry& registry, Entity entity, std::optional<EntityStateIndex> previous_state, EntityStateIndex state_index) const;
	};
}