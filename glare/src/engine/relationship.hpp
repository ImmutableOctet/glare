#pragma once

#include "types.hpp"

namespace engine
{
	struct Relationship
	{
		protected:
			std::uint32_t child_count = 0;

			Entity parent = null;

			Entity first = null;
			Entity prev = null;
			Entity next = null;
		public:
			// Internal use only.
			Relationship(Entity parent=null);

			inline std::uint32_t children() const { return child_count; }
			inline bool has_children() const { return (children() > 0); }
			
			inline Entity get_parent() const { return parent; }

			static void set_parent(Registry& registry, Entity self, Entity parent);
			//void set_parent(Registry& registry, Entity self, Entity parent);

			Entity add_child(Registry& registry, Entity self, Entity child);
			Entity remove_child(Registry& registry, Entity child, Entity self=null, bool remove_in_registry=false);

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

					auto next_child = relationship.next;

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
			Entity forward_previous(Registry& registry, bool remove_next=true);

			Relationship& collapse_child(Registry& registry, Relationship& child_relationship, Entity self=null);

			static Relationship& append_child(Relationship& prev_rel, Relationship& current_rel, Entity child, Entity next_child);

			// Returns 'null' if a 'Relationship' object does not already exist.
			static Relationship* remove_previous_parent(Registry& registry, Entity entity);
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