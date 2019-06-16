#include "entity.hpp"

namespace engine
{
	Entity::Entity(Entity* parent)
		: parent(parent) {}

	Entity* Entity::get_parent()
	{
		return parent;
	}
}