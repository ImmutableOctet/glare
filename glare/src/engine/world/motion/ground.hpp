#pragma once

#include <engine/world/physics/collision_events.hpp>

class btCollisionObject;

namespace engine
{
	class World;
	
	// Data describing a ground surface's properties.
	struct Ground : public CollisionSurface
	{
		public:
			bool is_dynamic : 1 = false;

			Ground() = default;
			Ground(const Ground&) = default;
			Ground(Ground&&) noexcept = default;

			inline Ground(const CollisionSurface& contact)
				: CollisionSurface(contact) {}

			inline Ground(World& world, const CollisionSurface& contact)
				: Ground(contact)
			{
				on_new_surface();
				update_metadata(world);
			}

			inline Ground& operator=(const CollisionSurface& surface)
			{
				CollisionSurface::operator=(surface);

				on_new_surface();

				return *this;
			}

			Ground& operator=(const Ground&) = default;
			Ground& operator=(Ground&&) noexcept = default;

			// The entity contacting the ground.
			inline Entity contacting_entity() const
			{
				return collision.a;
			}

			// An entity representing the contacted ground.
			inline Entity entity() const
			{
				return collision.b;
			}

			inline const btCollisionObject* collision_object() const
			{
				return collision.native.b_object;
			}

			// Same behavior as `entity`.
			inline operator Entity() const
			{
				return entity();
			}

			void update_metadata(World& world);
		private:
			void on_new_surface();
	};
}