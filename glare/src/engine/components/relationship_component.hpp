#pragma once

#include "types.hpp"

#include <entt/entity/registry.hpp>

#include <util/log.hpp>

#include <utility>
#include <type_traits>
//#include <optional>

namespace engine
{
	class World;
	class EntityFactory;

	struct RelationshipComponent
	{
		public:
			friend World;
			friend EntityFactory;

		protected:
			std::uint32_t child_count = 0;

			Entity parent = null;

			Entity first = null;
			Entity prev = null;
			Entity next = null;

		public:
			// This returns the previous parent entity, if it existed before a new assignment took place.
			static Entity set_parent(Registry& registry, Entity self, Entity parent, bool fail_on_existing_parent=false);
			//Entity set_parent(Registry& registry, Entity self, Entity parent);

			// If `child_relationship` is not specified, a lookup will be performed on `child`.
			Entity add_child(Registry& registry, Entity self, Entity child, RelationshipComponent* child_relationship = nullptr);
			Entity remove_child(Registry& registry, Entity child, Entity self = null, bool remove_in_registry = false);

			// Internal use only.
			// 
			// TODO: Review why this isn't currently protected or private.
			RelationshipComponent(Entity parent=null);

			// TODO: Review why these aren't protected/private:
			RelationshipComponent(const RelationshipComponent&) = default;
			RelationshipComponent& operator=(const RelationshipComponent&) = default;

			RelationshipComponent(RelationshipComponent&&) noexcept = default;
			RelationshipComponent& operator=(RelationshipComponent&&) noexcept = default;

			inline std::uint32_t children() const { return child_count; }
			inline bool has_children() const { return (children() > 0); }
			
			inline Entity get_parent() const { return parent; }

			std::tuple<Entity, RelationshipComponent*> get_first_child(Registry& registry) const;
			std::tuple<Entity, RelationshipComponent*> get_last_child(Registry& registry) const;

			// TODO: Finish documenting this overload.
			// 
			// Enumerates children, calling `fn` on each child.
			// `recursive` allows for traversal of the entire relationship tree.
			// `on_exit` is called once recursion has completed, or immediately after `fn` if recursion is disabled.
			template
			<
				typename response_type, // = decltype(enum_fn(std::declval<Entity>(), std::declval<RelationshipComponent&>(), std::declval<Entity>()))
				typename enum_fn,
				typename exit_fn,
				typename get_continuation_fn
			>
			inline Entity enumerate_children(Registry& registry, enum_fn&& fn, bool recursive, exit_fn on_exit, get_continuation_fn get_continuation, const response_type* parent_response=nullptr) const // enter_fn on_enter
			{
				auto child = first;

				while (child != null)
				{
					auto& relationship = registry.get<RelationshipComponent>(child); // *this;

					auto next_child = relationship.next; // const auto&

					if (next_child == child)
					{
						break;
					}

					//on_enter();

					response_type response = fn(child, relationship, next_child, parent_response);

					auto continue_recurse = get_continuation(response);

					if (continue_recurse)
					{
						if (recursive)
						{
							auto result = relationship.enumerate_children(registry, fn, true, on_exit, get_continuation, &response);

							if (result != null)
							{
								return result;
							}
						}
					}

					on_exit(child, relationship, next_child, response);

					if (!continue_recurse)
					{
						break;
					}

					//print("Going from {} to {}", child, next_child);

					child = next_child;
				}

				return child;
			}

			// `fn` must return a boolean value, or a value convertible to one.
			// The `fn` callable's signature must be:
			// `bool fn(Entity child, [const] RelationshipComponent& child_relationship, Entity next_child)` - or similar.
			template <typename enum_fn, typename exit_fn, typename response_type=bool> // typename enter_fn
			inline Entity enumerate_children(Registry& registry, enum_fn&& fn, bool recursive, exit_fn on_exit) const
			{
				return enumerate_children<response_type>
				(
					registry,
					
					[&fn](auto child, auto& relationship, auto next_child, auto* parent_response)
					{
						return fn(child, relationship, next_child);
					},
					
					recursive,
					on_exit,

					[](auto continue_recurse)
					{
						return static_cast<bool>(continue_recurse);
					}
				);
			}

			// `fn` must return a boolean value, or a value convertible to one.
			// The `fn` callable's signature must be:
			// `bool fn(Entity child, [const] RelationshipComponent& child_relationship, Entity next_child)` - or similar.
			template <typename enum_fn, typename response_type=bool>
			inline Entity enumerate_children(Registry& registry, enum_fn&& fn, bool recursive=false) const
			{
				return enumerate_children
				(
					registry,
					std::forward<enum_fn>(fn),
					recursive,
					[](auto child, auto& relationship, auto next_child, auto response) {}
				);
			}

			// Enumerates children and adds them to `out` via `push_back`.
			// NOTE: This routine is intended for `std::vector`-like containers.
			template <typename Container>
			inline std::uint32_t get_children(Registry& registry, Container& out, bool recursive=false) const
			{
				std::uint32_t child_count = 0;

				enumerate_children(registry, [&out, &child_count](Entity child, RelationshipComponent& relationship, Entity next_child)
				{
					out.push_back(static_cast<typename Container::value_type>(child));

					child_count++;

					return true;
				}, recursive);

				return child_count;
			}

			// Retrieves an `std::vector` of child entities.
			// If populating an existing container, please use the templated container-based overload.
			inline auto get_children(Registry& registry, bool recursive=false) const
			{
				std::vector<Entity> out;

				get_children(registry, out, recursive);

				return out;
			}

			// Simple single-level entity enumeration of this relationship's children.
			// For a more robust (and optionally recursive) enumeration routine, see `enumerate_children`.
			// 
			// The `fn` parameter is a callable that must return a boolean indicating whether to continue enumeration.
			// If enumeration is short-circuited, the entity returned is the last entity sent to `fn`.
			// If enumeration completes, the returned entity will be `null`.
			template <typename enum_fn>
			inline Entity enumerate_child_entities(Registry& registry, enum_fn&& fn) const
			{
				auto child = first;

				while (child != null)
				{
					auto& relationship = registry.get<RelationshipComponent>(child); // *this;

					auto next_child = relationship.next;

					if (child == next_child)
					{
						break;
					}

					if (!fn(child, next_child))
					{
						break;
					}

					child = next_child;
				}

				return child;
			}

			// Reports the total number of children, including children of children, etc.
			inline std::uint32_t total_children(Registry& registry) const
			{
				std::uint32_t count = 0;

				enumerate_children(registry, [&registry, &count](Entity child, const RelationshipComponent& relationship, Entity next_child)
				{
					count++;

					count += relationship.total_children(registry);

					return true;
				});

				return count;
			}
		protected:
			// Returns the 'Entity' identifier representing this object.
			Entity forward_previous(Registry& registry);

			RelationshipComponent& collapse_child(Registry& registry, RelationshipComponent& child_relationship, Entity self, Entity child);

			static RelationshipComponent& append_child(RelationshipComponent& prev_rel, RelationshipComponent& current_rel, Entity child, Entity next_child);

			// Returns 'null' if a 'RelationshipComponent' object does not already exist.
			// `entity_relationship` is optional; if not specified, a lookup will take place.
			static RelationshipComponent* remove_previous_parent(Registry& registry, Entity entity, RelationshipComponent* entity_relationship=nullptr);
		public:
			template <typename ParentFn>
			inline std::size_t enumerate_parents(Registry& registry, ParentFn&& enum_fn)
			{
				auto parent_entity = get_parent();

				std::size_t count = 0;

				while (parent_entity != null)
				{
					auto& parent_relationship = registry.get<RelationshipComponent>(parent_entity);

					if constexpr (std::is_invocable_r_v<bool, ParentFn, Entity, RelationshipComponent&, std::size_t> || std::is_invocable_r_v<bool, ParentFn, Entity, const RelationshipComponent&, std::size_t>)
					{
						if (!enum_fn(parent_entity, parent_relationship, ++count))
						{
							break;
						}
					}
					else if constexpr (std::is_invocable_v<bool, ParentFn, Entity, RelationshipComponent&, std::size_t> || std::is_invocable_r_v<bool, ParentFn, Entity, const RelationshipComponent&, std::size_t>)
					{
						enum_fn(parent_entity, parent_relationship, ++count);
					}
					else if constexpr (std::is_invocable_r_v<bool, ParentFn, Entity, RelationshipComponent&> || std::is_invocable_r_v<bool, ParentFn, Entity, const RelationshipComponent&>)
					{
						if (!enum_fn(parent_entity, parent_relationship))
						{
							break;
						}
					}
					else if constexpr (std::is_invocable_v<ParentFn, Entity, RelationshipComponent&> || std::is_invocable_v<ParentFn, Entity, const RelationshipComponent&>)
					{
						enum_fn(parent_entity, parent_relationship);
					}
					else if constexpr (std::is_invocable_r_v<bool, ParentFn, Entity>)
					{
						if (!enum_fn(parent_entity))
						{
							break;
						}
					}
					else // if constexpr (std::is_invocable_v<ParentFn, Entity>)
					{
						enum_fn(parent_entity);
					}

					parent_entity = parent_relationship.get_parent();
				}

				return count;
			}

			template <typename ParentFn>
			inline std::size_t enumerate_parents(Registry& registry, ParentFn&& enum_fn) const
			{
				return const_cast<RelationshipComponent*>(this)->enumerate_parents(registry, std::forward<ParentFn>(enum_fn));
			}
	};
}