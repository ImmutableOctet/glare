#pragma once

#include "mesh.hpp"

//#include <types.hpp>
//#include "types.hpp"

#include <memory>
#include <vector>

// Required for `std::unique_ptr`'s destructor:
//#include <bullet/BulletCollision/CollisionShapes/btTriangleMesh.h>
#include <bullet/btBulletCollisionCommon.h>

class btTriangleMesh;
struct btIndexedMesh;

namespace graphics
{
	// A type representing a uniquely allocated Bullet `mesh_interface` (handle/descriptor)
	// and its associated `mesh_data` (vertices). This is a move-only type.
	struct CollisionGeometry // CollisionData
	{
		public:
			using Descriptor = btTriangleMesh; // btTriangleIndexVertexArray;
			using Container = std::vector<SimpleMeshData>;

			CollisionGeometry(Container&& mesh_data);
			CollisionGeometry(CollisionGeometry&&) noexcept = default;

			//~CollisionGeometry();

			CollisionGeometry& operator=(CollisionGeometry&&) noexcept = default;

			std::unique_ptr<Descriptor> mesh_interface;
			Container mesh_data;

			bool has_32bit_indices() const;
		protected:
			std::unique_ptr<Descriptor> generate_mesh_interface();
	};
}