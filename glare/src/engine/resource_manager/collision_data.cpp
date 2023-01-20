#include "collision_data.hpp"

#include <engine/world/physics/collision_shape_description.hpp>

#include <math/bullet.hpp>

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

	CollisionShape CollisionData::build_basic_shape(const CollisionShapeDescription& shape_details)
	{
		switch (shape_details.primitive)
		{
			case CollisionShapePrimitive::Capsule:
				return std::static_pointer_cast<CollisionRaw>(std::make_shared<btCapsuleShape>(shape_details.get_xz_size(), shape_details.get_height())); // shape_details.get_radius()

			case CollisionShapePrimitive::Cube:
				return std::static_pointer_cast<CollisionRaw>(std::make_shared<btBoxShape>(math::to_bullet_vector(shape_details.size)));

			case CollisionShapePrimitive::Sphere:
				return std::static_pointer_cast<CollisionRaw>(std::make_shared<btSphereShape>(shape_details.get_radius()));

			/*
			default:
				// Unsupported shape primitive.
				assert(false);

				break;
			*/
		}

		return {};
	}

	CollisionData::CollisionData(const Shape& collision_shape)
		: collision_shape(collision_shape) {}

	CollisionData::CollisionData(Geometry&& geometry_storage, bool optimize)
		: collision_shape(build_mesh_shape(geometry_storage, optimize)), geometry_storage(std::move(geometry_storage)) {}

	CollisionData::CollisionData(const CollisionShapeDescription& shape_details)
		: collision_shape(build_basic_shape(shape_details)) {}
}