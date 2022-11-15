#pragma once

#include "collision_events.hpp"
//#include "collision_surface.hpp"

class btCollisionObject;

namespace engine
{
	class World;
	
	// Data describing a ground surface's properties.
	struct Ground : public CollisionSurface
	{
		public:
			bool is_dynamic : 1 = false;

			// Used primarily by external systems; updates automatically based on
			// whether a collision object can be resolved from a contact-surface.
			// 
			// NOTE: This can be further changed by an external source. (e.g. for ground detection)
			bool is_contacted : 1 = true;

			Ground() = default;
			Ground(const Ground&) = default;
			Ground(Ground&&) noexcept = default;

			inline Ground(const CollisionSurface& contact)
				: CollisionSurface(contact)
			{
				on_new_surface();
			}

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

			inline bool get_is_dynamic() const { return is_dynamic; }
			inline void set_is_dynamic(bool value) { is_dynamic = value; }

			inline bool get_is_contacted() const { return is_contacted; }
			inline void set_is_contacted(bool value) { is_contacted = value; }

			void update_metadata(World& world);
		private:
			void on_new_surface();
	};
}