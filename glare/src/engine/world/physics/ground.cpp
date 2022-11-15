#include "ground.hpp"

#include <engine/world/world.hpp>
#include <bullet/btBulletCollisionCommon.h>

namespace engine
{
	void Ground::update_metadata(World& world)
	{
		//auto& registry = world.get_registry();

		// ...
	}

	void Ground::on_new_surface()
	{
		auto ground_obj = collision_object();

		is_dynamic = false;

		if (ground_obj)
		{
			if (!ground_obj->isStaticObject() || ground_obj->isKinematicObject())
			{
				is_dynamic = true;
			}

			is_contacted = true;
		}
		else
		{
			is_contacted = false;
		}
	}
}