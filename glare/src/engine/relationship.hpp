#pragma once

#include "types.hpp"

namespace engine
{
	class World;

	struct Relationship
	{
		public:
			friend World;
		protected:
			std::uint32_t child_count = 0;

			Entity parent = null;

			Entity first = null;
			Entity prev = null;
			Entity next = null;

			// This returns the previous parent entity, if it existed before a new assignment took place.
			static Entity set_parent(Registry& registry, Entity self, Entity parent);
			//Entity set_parent(Registry& registry, Entity self, Entity parent);

			// If `child_relationship` is not specified, a lookup will be performed on `child`.
			Entity add_child(Registry& registry, Entity self, Entity child, Relationship* child_relationship = nullptr);
			Entity remove_child(Registry& registry, Entity child, Entity self = null, bool remove_in_registry = false);
		public:
			// Internal use only.
			Relationship(Entity parent=null);

			Relationship(Relationship&&) = default;
			Relationship& operator=(Relationship&&) = default;

			Relationship(const Relationship&) = default;
			Relationship& operator=(const Relationship&) = default;

			inline std::uint32_t children() const { return child_count; }
			inline bool has_children() const { return (children() > 0); }
			
			inline Entity get_parent() const { return parent; }

			std::tuple<Entity, Relationship*> get_first_child(Registry& registry) const;
			std::tuple<Entity, Relationship*> get_last_child(Registry& registry) const;

			// Enumerates children, returning the 'Entity' stopped at by 'fn'.
			template <typename enum_fn>
			inline Entity enumerate_children(Registry& registry, enum_fn fn) const
			{
				auto child = first;

				while (child != null)
				{
					auto& relationship = registry.get<Relationship>(child); // *this;

					auto next_child = relationship.next; // const auto&

					if (next_child == child)
					{
						break;
					}

					if (!fn(child, relationship, next_child))
					{
						break;
					}

					child = next_child;
				}

				return child;
			}

			template <typename enum_fn>
			inline Entity enumerate_child_entities(Registry& registry, enum_fn fn) const
			{
				auto child = first;

				while (child != null)
				{
					auto& relationship = registry.get<Relationship>(child); // *this;
					auto next_child = relationship.next;

					if (!fn(child, next_child))
					{
						break;
					}

					child = next_child;
				}

				return child;
			}
		protected:
			// Returns the 'Entity' identifier representing this object.
			Entity forward_previous(Registry& registry);

			Relationship& collapse_child(Registry& registry, Relationship& child_relationship, Entity self, Entity child);

			static Relationship& append_child(Relationship& prev_rel, Relationship& current_rel, Entity child, Entity next_child);

			// Returns 'null' if a 'Relationship' object does not already exist.
			// `entity_relationship` is optional; if not specified, a lookup will take place.
			static Relationship* remove_previous_parent(Registry& registry, Entity entity, Relationship* entity_relationship=nullptr);
		public:
			template <typename enum_fn>
			inline std::uint32_t enumerate_parents(Registry& registry, enum_fn)
			{
				auto entity = get_parent();

				std::uint32_t count = 0;

				while (entity != null)
				{
					auto& relationship = registry.get<Relationship>(entity);

					enum_fn(entity, relationship, ++count);

					entity = relationship.get_parent();
				}

				return count;
			}
	};
}