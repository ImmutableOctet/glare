#include "collision_data.hpp"

#include <graphics/collision_geometry.hpp>
#include <bullet/btBulletCollisionCommon.h>

#include <memory>

namespace engine
{
	CollisionShape CollisionData::build_mesh_shape(const CollisionData::Geometry& geometry_storage, bool optimize)
	{
		auto desc = geometry_storage.mesh_interface.get(); // auto*

		assert(desc);

		auto shape = std::make_shared<btBvhTriangleMeshShape>(desc, optimize); // std::shared_ptr<btTriangleMeshShape>

		// Return the generated `shape` object as a pointer to a generic shape, rather than as a triangle-mesh shape. (Internally a `btCollisionShape` type)
		return std::static_pointer_cast<CollisionRaw>(shape);
	}

	CollisionData::CollisionData(const Shape& collision_shape)
		: collision_shape(collision_shape) {}

	CollisionData::CollisionData(Geometry&& geometry_storage, bool optimize)
		: collision_shape(build_mesh_shape(geometry_storage, optimize)), geometry_storage(std::move(geometry_storage)) {}
}