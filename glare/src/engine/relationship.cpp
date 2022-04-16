#include "relationship.hpp"
#include "transform.hpp"

#include <math/math.hpp>

// Debugging related:
#include <util/log.hpp>

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

	Entity Relationship::set_parent(Registry& registry, Entity self, Entity parent)
	{
		assert(self != null);

		if ((self == parent) || (parent == null))
		{
			return null;
		}

		//auto _test_ent = static_cast<Entity>(37);
		//auto* _test_rel = (registry.valid(_test_ent)) ? registry.try_get<Relationship>(_test_ent) : nullptr;

		auto* prev_rel = registry.try_get<Relationship>(self);
		Entity prev_parent = null;

		if (prev_rel)
		{
			prev_parent = prev_rel->parent;
		}

		//print("I am: {}, my previous parent is: {}, and my new parent is going to be: {}", self, prev_parent, parent);

		auto& parent_relationship = registry.get_or_emplace<Relationship>(parent);

		parent_relationship.add_child(registry, parent, self, prev_rel);

		//registry.replace<Relationship>(parent, std::move(parent_relationship)); // [&](auto& r) { r = std::move(parent_relationship); }

		return prev_parent;
	}

	Entity Relationship::add_child(Registry& registry, Entity self, Entity child, Relationship* child_relationship)
	{
		assert(self != null);

		if ((self == child) || (child == null))
		{
			return null;
		}

		bool already_has_child = false;

		enumerate_children(registry, [&](auto current, auto& relation, auto next)
		{
			if (current == child)
			{
				already_has_child = true;

				return false;
			}

			return true;
		});

		if (already_has_child)
		{
			return child;
		}

		auto* prev_rel = remove_previous_parent(registry, child, child_relationship);

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

			assert(last_child != null);

			auto& last_child_rel = *std::get<1>(last);

			append_child(last_child_rel, relationship, last_child, child);
		}

		math::Matrix current_matrix;

		std::optional<Transform> child_transform = Transform::get_transform_safe(registry, child);

		if (child_transform)
		{
			if (std::optional<Transform> self_transform = Transform::get_transform_safe(registry, self))
			{
				current_matrix = self_transform->get_inverse_matrix() * child_transform->get_local_matrix();
			}
		}

		/*
		if ((self != null) && (((int)self) != 0))
		{
			print("Adding child ({}) to parent ({})", (int)child, (int)self);

			//_dbg_is_actual_add = true;
		}
		*/

		registry.emplace_or_replace<Relationship>(child, std::move(relationship));

		this->child_count++;

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
			//auto con = util::log::get_console();
			////std::cout << "Removing child (" << (int)child << ") from parent (" << (int)self << ")\n";

			//_dbg_is_actual_removal = true;
		}

		auto child_relationship = registry.get<Relationship>(child);

		///*
		std::optional<math::Matrix> current_matrix = std::nullopt;

		if (std::optional<Transform> child_transform = Transform::get_transform_safe(registry, child))
		{
			current_matrix = child_transform->get_matrix();
		}
		//*/

		collapse_child(registry, child_relationship, self, child);

		if (remove_in_registry)
		{
			registry.remove<Relationship>(child);
		}
		else
		{
			registry.replace<Relationship>(child, std::move(child_relationship)); // [&](auto& r) { r = child_relationship; }
		}

		///*
		if (current_matrix)
		{
			if (std::optional<Transform> child_transform = Transform::get_transform_safe(registry, child))
			{
				//if (_dbg_is_actual_removal)
				//{
				print("Re-applying matrix...");
				//}

				child_transform->set_matrix(*current_matrix);
			}
		}
		//*/

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

	Entity Relationship::forward_previous(Registry& registry)
	{
		if (this->prev == null)
		{
			if (this->next != null)
			{
				auto& next_rel = registry.get<Relationship>(this->next);

				next_rel.prev = this->prev; // null;

				this->next = null;
			}

			return null;
		}

		auto& previous_rel = registry.get<Relationship>(this->prev);

		auto self = previous_rel.next;

		previous_rel.next = this->next;

		this->next = null;
		this->prev = null;

		return self;
	}

	Relationship& Relationship::collapse_child(Registry& registry, Relationship& child_relationship, Entity self, Entity child)
	{
		//assert(registry.try_get<Relationship>(child));

		//auto& child_relationship = registry.get<Relationship>(child);

		//assert((self == null) || (child_relationship.parent == self));

		auto next_child = child_relationship.next;

		child_relationship.forward_previous(registry);
		child_relationship.parent = null;

		// If this was the first child, set the new "first child"
		// to the next child after the one we're collapsing.
		// ('null' if no other children remain)
		if (this->first == child)
		{
			this->first = next_child;
		}

		this->child_count--;

		return child_relationship;
	}

	Relationship& Relationship::append_child(Relationship& prev_rel, Relationship& current_rel, Entity prev_child, Entity next_child)
	{
		prev_rel.next = next_child;
		current_rel.prev = prev_child;

		return current_rel;
	}

	// Internal subroutine.
	Relationship* Relationship::remove_previous_parent(Registry& registry, Entity entity, Relationship* entity_relationship)
	{
		auto* relationship = entity_relationship;
		
		if (!relationship)
		{
			relationship = registry.try_get<Relationship>(entity);

			if (!relationship)
			{
				return nullptr;
			}
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