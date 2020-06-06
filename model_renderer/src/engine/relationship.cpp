#include "relationship.hpp"

namespace engine
{
	Relationship::Relationship(Entity parent)
		: parent(parent), first(null), prev(null), next(null) {}

	Entity Relationship::add_child(Registry& registry, Entity self, Entity child)
	{
		auto* prev_rel = remove_previous_parent(registry, child);

		Relationship relationship = Relationship(self);

		bool is_first = (child_count == 0); // (first == null);

		if (is_first)
		{
			first = child;
		}
		else
		{
			auto last = get_last_child(registry);
			auto last_child = std::get<0>(last);

			ASSERT(last_child != null);

			auto& last_child_rel = *std::get<1>(last);

			append_child(last_child_rel, relationship, last_child, child);
		}

		registry.assign_or_replace<Relationship>(child, relationship);

		child_count++;

		return child;
	}

	Entity Relationship::remove_child(Registry& registry, Entity child, Entity self, bool remove_in_registry)
	{
		collapse_child(registry, child, self);

		if (remove_in_registry)
		{
			registry.remove<Relationship>(child);
		}

		return child;
	}

	std::tuple<Entity, Relationship*> Relationship::get_first_child(Registry& registry) const
	{
		auto child = first;

		if (child == null)
		{
			return {}; // { null, nullptr };
		}

		auto& relationship = registry.get<Relationship>(child);

		return { child, &relationship };
	}

	std::tuple<Entity, Relationship*> Relationship::get_last_child(Registry& registry) const
	{
		Relationship* relationship = nullptr;

		auto child = enumerate_children(registry, [&](auto current, auto& relation, auto next)
			{
				if (next == null)
				{
					relationship = &relation;

					return false;
				}

				return true;
			});

		return { child, relationship };
	}

	Entity Relationship::forward_previous(Registry& registry, bool remove_next)
	{
		if (prev == null)
		{
			return null;
		}

		auto& previous_rel = registry.get<Relationship>(prev);

		auto self = previous_rel.next;

		previous_rel.next = this->next;

		if (remove_next)
		{
			this->next = null;
		}

		return self;
	}

	// Functions:
	Relationship& Relationship::collapse_child(Registry& registry, Entity child, Entity self)
	{
		ASSERT(registry.has<Relationship>(child));

		auto& relationship = registry.get<Relationship>(child);

		ASSERT((self == null) || (relationship.parent == self));

		relationship.forward_previous(registry);

		child_count--;

		return relationship;
	}

	Relationship& Relationship::append_child(Relationship& prev_rel, Relationship& current_rel, Entity prev_child, Entity next_child)
	{
		prev_rel.next = next_child;
		current_rel.prev = prev_child;

		return current_rel;
	}

	Relationship* Relationship::remove_previous_parent(Registry& registry, Entity entity)
	{
		auto* relationship = registry.try_get<Relationship>(entity);

		if (relationship == nullptr)
		{
			return nullptr;
		}

		auto parent = relationship->parent;

		if (parent == null)
		{
			return nullptr;
		}

		auto& parent_relationship = registry.get<Relationship>(parent);

		// Standard removal method:
		parent_relationship.remove_child(registry, entity, parent, false);

		return relationship;
	}
}