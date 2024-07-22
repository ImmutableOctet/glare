#pragma once

#include "types.hpp"

#include <engine/meta/types.hpp>
#include <engine/meta/meta_any.hpp>
#include <engine/meta/meta_type.hpp>
#include <engine/meta/resolve.hpp>
#include <engine/meta/traits.hpp>

//#include <engine/meta/short_name.hpp>

//#include <entt/meta/meta.hpp>

//#include <util/small_vector.hpp>

#include <vector>

#include <algorithm>
#include <type_traits>
#include <utility>

#include <memory>

namespace engine
{
	class SystemManagerInterface
	{
		public:
			using SystemID = MetaTypeID;

			struct System
			{
				SystemID type_id;
				MetaAny ptr;

				inline MetaType type() const
				{
					return resolve(type_id);
				}

				inline System as_ref()
				{
					return { type_id, ptr.as_ref() };
				}

				inline System as_ref() const
				{
					return { type_id, ptr.as_ref() };
				}

				template <typename SystemType>
				auto* get()
				{
					return try_get<SystemType>(ptr);
				}

				template <typename SystemType>
				auto* get() const
				{
					return try_get<SystemType>(ptr);
				}

				inline explicit operator bool() const
				{
					return static_cast<bool>(ptr);
				}
			};

			using SystemCollection    = std::vector<System>; // util::small_vector<System, 32>;
			using SystemIterator      = SystemCollection::iterator;
			using ConstSystemIterator = SystemCollection::const_iterator;

		protected:
			template <typename SystemType, typename OpaqueType>
			static auto* try_get(OpaqueType&& system_ptr)
			{
				auto handle = system_ptr.try_cast<std::unique_ptr<SystemType>>();

				if (handle)
				{
					return handle->get();
				}

				return decltype(handle->get()) {}; // {};
			}

			template <typename SystemType>
			static SystemID key() // constexpr
			{
				//return short_name_hash<SystemType>().value();
				//return entt::type_hash<SystemType>::value();

				//return entt::type_hash<std::unique_ptr<SystemType>>::value();

				const auto type = resolve<SystemType>();

				assert(type);

				if (type)
				{
					return type.id();
				}

				return SystemID {};
			}

			template <typename Collection>
			static auto get_system_iterator(Collection& systems, SystemID system_id)
			{
				return std::find_if
				(
					systems.begin(), systems.end(),

					[system_id](const System& system_entry)
					{
						if (system_entry.type_id == system_id)
						{
							return true;
						}

						return false;
					}
				);
			}

			template <template<typename, typename, typename...> typename TraitType, typename SystemType, typename ServiceType>
			static constexpr bool has_method()
			{
				using Type = std::decay_t<SystemType>;

				if constexpr (TraitType<Type, bool, ServiceType&>::value)
				{
					return true;
				}
				else if constexpr (TraitType<Type, bool, Service&>::value)
				{
					return true;
				}

				//return (std::is_base_of_v<WorldSystem, SystemType> && std::is_base_of_v<World, ServiceType>);

				return false;
			}

			template <typename SystemType, typename ServiceType>
			static constexpr bool has_subscribe()
			{
				return has_method<engine::has_method_subscribe, SystemType, ServiceType>();
			}

			template <typename SystemType, typename ServiceType>
			static constexpr bool has_unsubscribe()
			{
				//return has_subscribe<SystemType>();
				return has_method<engine::has_method_unsubscribe, SystemType, ServiceType>();
			}

			// Emplaces a new instance of `SystemType`, if one does not already exist.
			// If an instance of `SystemType` is found, a reference to that object will be returned instead.
			template <typename SystemType, typename...Args>
			SystemType& emplace_system(Args&&... args)
			{
				const auto type_id = key<SystemType>();

				auto existing = get_system_iterator(systems, type_id);

				if (existing != systems.end())
				{
					auto ptr = existing->get<SystemType>();

					assert(ptr);

					return *ptr;
				}

				auto allocated_system = std::make_unique<SystemType>(std::forward<Args>(args)...);

				auto allocated_ptr = allocated_system.get();

				assert(allocated_ptr);

				auto& system = systems.emplace_back
				(
					type_id,

					MetaAny { std::move(allocated_system) }
				);

				return *allocated_ptr;
			}

			// Destroys a system instance, unsubscribing it from `service`, if possible.
			// This function returns true if the system was successfully destroyed.
			template <typename SystemType, typename ServiceType> // ServiceType=Service
			bool destroy_system(ServiceType& service)
			{
				auto it = get_system_iterator(systems, key<SystemType>());

				if (it == systems.end())
				{
					// System not found.
					return false;
				}

				if constexpr (has_unsubscribe<SystemType, ServiceType>())
				{
					if (auto system_ptr = it->get<SystemType>())
					{
						// Guarantee that the system unsubscribes from `service`, prior to destruction.
						system_ptr->unsubscribe(service);
					}
				}

				// Remove the underlying 'system' object internally.
				systems.erase(it);

				// System destroyed.
				return true;
			}

			// Opaque container of systems and their resources.
			SystemCollection systems;

		public:
			/*
				TODO: Look into scenarios where the underlying object of `system` could be moved as well.
				Such scenarios are usually unsafe, since the underlying object is likely subscribed to one or more event sinks.

				Moves `system` into an internal container of active `System` objects,
				then returns a reference using the new memory location within the container.
				
				This routine does not handle subscription for the `system` object.
				
				For general usage, see `emplace_system`.
			*/
			System& add_system(System&& system, bool check_existing=true, bool override_existing=false)
			{
				if (check_existing)
				{
					for (auto& existing : systems)
					{
						if (existing.type_id == system.type_id)
						{
							if (override_existing)
							{
								existing = std::move(system);
							}

							return existing;
						}
					}
				}

				return systems.emplace_back(std::move(system));
			}

			// This routine does not handle subscription for the `system` object.
			// For general usage, see `emplace_system`.
			template
			<
				typename SystemType,
				typename = std::enable_if_t<!std::is_same_v<std::decay_t<SystemType>, System>>
			>
			SystemType& add_system(SystemType&& native_system, bool check_existing=true, bool override_existing=false)
			{
				const auto system_type_id = key<SystemType>();

				if (check_existing)
				{
					for (auto& existing : systems)
					{
						if (existing.type_id == system_type_id)
						{
							if (override_existing)
							{
								auto allocated_system = std::make_unique<SystemType>(std::move(native_system));
								auto allocated_system_raw = allocated_system.get();

								assert(allocated_system_raw);

								existing.ptr = std::move(allocated_system);

								return *allocated_system_raw;
							}

							auto existing_system_raw = existing.get<SystemType>();

							assert(existing_system_raw);

							return *existing_system_raw;
						}
					}
				}

				auto newly_allocated_system = std::make_unique<SystemType>(std::move(native_system));
				auto newly_allocated_system_raw = newly_allocated_system.get();

				systems.emplace_back
				(
					system_type_id,

					MetaAny { std::move(newly_allocated_system) }
				);

				return *newly_allocated_system_raw;
			}

			// Retrieves an instance of `SystemType`, if one exists, as well as its iterator in the internal container.
			// If a `SystemType` object has not been registered, this will return `nullptr`.
			template <typename SystemType>
			std::tuple<SystemIterator, SystemType*> get_system_ex()
			{
				auto it = get_system_iterator(systems, key<SystemType>());

				if (it == systems.end())
				{
					return { systems.end(), nullptr };
				}

				auto& system = *it;

				auto ptr = system.get<SystemType>();

				if (!ptr)
				{
					return { systems.end(), nullptr };
				}

				return { std::move(it), ptr };
			}

			// Retrieves an instance of `SystemType`, if one exists, as well as its iterator in the internal container.
			// If a `SystemType` object has not been registered, this will return `nullptr`.
			template <typename SystemType>
			std::tuple<ConstSystemIterator, const SystemType*> get_system_ex() const
			{
				const auto it = get_system_iterator(systems, key<SystemType>());

				if (it == systems.end())
				{
					return { systems.end(), nullptr };
				}

				const auto& system = *it;

				const auto ptr = system.get<SystemType>();

				if (!ptr)
				{
					return { systems.end(), nullptr };
				}

				return { std::move(it), ptr };
			}

			// Retrieves an instance of `SystemType`, if one exists.
			// If a `SystemType` object has not been registered, this will return `nullptr`.
			template <typename SystemType>
			SystemType* get_system()
			{
				return std::get<1>(get_system_ex<SystemType>());
			}

			template <typename SystemType>
			const SystemType* get_system() const
			{
				return std::get<1>(get_system_ex<SystemType>());
			}

			inline System get_system_handle(SystemID system_id)
			{
				if (auto it = get_system_iterator(systems, system_id); it != systems.end())
				{
					return it->as_ref();
				}

				return {};
			}

			inline System get_system_handle(SystemID system_id) const
			{
				if (auto it = get_system_iterator(systems, system_id); it != systems.cend())
				{
					return it->as_ref();
				}

				return {};
			}

			inline auto begin_iterator() { return systems.begin(); }
			inline auto begin() { return begin_iterator(); }

			inline auto end_iterator() { return systems.end(); }
			inline auto end() { return end_iterator(); }
	};
}