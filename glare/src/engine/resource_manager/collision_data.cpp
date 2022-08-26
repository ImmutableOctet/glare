#include "collision_data.hpp"

#include <memory>

namespace engine
{
	CollisionShape CollisionData::build_mesh_shape(const CollisionGeometry& geometry_storage, bool optimize)
	{
		auto desc = geometry_storage.mesh_interface.get(); // auto*

		assert(desc);

		auto shape = std::make_shared<btBvhTriangleMeshShape>(desc, optimize); // std::shared_ptr<btTriangleMeshShape>

		return std::static_pointer_cast<CollisionRaw>(shape);
	}

	CollisionData::CollisionData(const Shape& collision_shape)
		: collision_shape(collision_shape) {}

	CollisionData::CollisionData(Geometry&& geometry_storage, bool optimize)
		: collision_shape(build_mesh_shape(geometry_storage, optimize)), geometry_storage(std::move(geometry_storage)) {}
}