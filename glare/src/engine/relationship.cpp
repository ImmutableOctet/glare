#include "relationship.hpp"
#include "transform.hpp"

#include <math/math.hpp>

// Debugging related:
#include <iostream>

namespace engine
{
	Relationship::Relationship(Entity parent)
		: parent(parent), first(null), prev(null), next(null) {}

	/*
	void Relationship::set_parent(Registry& registry, Entity self, Entity parent)
	{
		auto parent_rel = registry.get_or_emplace<Relationship>(parent);

		parent_rel.add_child(registry, parent, self);

		registry.replace<Relationship>(parent, [&](auto& r) { r = parent_rel; });
	}
	*/

	void Relationship::set_parent(Registry& registry, Entity self, Entity parent)
	{
		auto parent_relationship = registry.get_or_emplace<Relationship>(parent);

		parent_relationship.add_child(registry, parent, self);

		registry.replace<Relationship>(parent, std::move(parent_relationship)); // [&](auto& r) { r = std::move(parent_relationship); }
	}

	Entity Relationship::add_child(Registry& registry, Entity self, Entity child)
	{
		auto* prev_rel = remove_previous_parent(registry, child);

		Relationship relationship;
		
		if (prev_rel != nullptr)
		{
			relationship = Relationship(*prev_rel);

			relationship.parent = self;
		}
		else
		{
			relationship = Relationship(self);
		}

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

		math::Matrix current_matrix;

		std::optional<Transform> child_transform = Transform::get_transform_safe(registry, child);

		if (child_transform)
		{
			current_matrix = child_transform->get_matrix();
		}

		if ((self != null) && (((int)self) != 0))
		{
			std::cout << "Adding child (" << (int)child << ") to parent (" << (int)self << ")\n";

			//_dbg_is_actual_add = true;
		}

		registry.emplace_or_replace<Relationship>(child, relationship);

		child_count++;

		if (child_transform)
		{
			child_transform->set_matrix(current_matrix);
		}

		return child;
	}

	Entity Relationship::remove_child(Registry& registry, Entity child, Entity self, bool remove_in_registry)
	{
		//bool _dbg_is_actual_removal = false;

		// Debugging related:
		if ((self != null) && (((int)self) != 0))
		{
			std::cout << "Removing child (" << (int)child << ") from parent (" << (int)self << ")\n";

			//_dbg_is_actual_removal = true;
		}

		auto child_relationship = registry.get<Relationship>(child);

		math::Matrix current_matrix;

		std::optional<Transform> child_transform = Transform::get_transform_safe(registry, child);

		if (child_transform)
		{
			current_matrix = child_transform->get_matrix();
		}

		collapse_child(registry, child_relationship, self); // ..., child, ...

		if (remove_in_registry)
		{
			registry.remove<Relationship>(child);
		}
		else
		{
			registry.replace<Relationship>(child, std::move(child_relationship)); // [&](auto& r) { r = child_relationship; }
		}

		if (child_transform)
		{
			//if (_dbg_is_actual_removal)
			//{
			std::cout << "Re-applying matrix...\n";
			//}

			child_transform->set_matrix(current_matrix);
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

	Relationship& Relationship::collapse_child(Registry& registry, Relationship& child_relationship, Entity self) // Entity child
	{
		//ASSERT(registry.has<Relationship>(child));

		//auto& child_relationship = registry.get<Relationship>(child);

		//ASSERT((self == null) || (child_relationship.parent == self));

		child_relationship.forward_previous(registry);

		child_count--;

		return child_relationship;
	}

	Relationship& Relationship::append_child(Relationship& prev_rel, Relationship& current_rel, Entity prev_child, Entity next_child)
	{
		prev_rel.next = next_child;
		current_rel.prev = prev_child;

		return current_rel;
	}

	// Internal subroutine.
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
			return relationship; // nullptr;
		}

		auto& parent_relationship = registry.get<Relationship>(parent);

		// Standard removal method:
		parent_relationship.remove_child(registry, entity, parent, false);

		return relationship;
	}
}