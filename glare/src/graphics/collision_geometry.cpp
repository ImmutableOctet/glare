// TODO: Move to a more appropriate submodule. (i.e. anywhere else but `graphics`)

#include "collision_geometry.hpp"

/*
#include <bullet/BulletCollision/CollisionShapes/btTriangleMesh.h>
#include <bullet/BulletCollision/CollisionShapes/btTriangleIndexVertexArray.h>
#include <bullet/BulletCollision/CollisionShapes/btConcaveShape.h>
*/

#include <bullet/btBulletCollisionCommon.h>

namespace graphics
{
	CollisionGeometry::CollisionGeometry(Container&& mesh_data)
		: mesh_interface(generate_mesh_interface()), mesh_data(std::move(mesh_data))
	{
		auto ph_index_type = (has_32bit_indices() ? PHY_INTEGER : PHY_SHORT);

		for (const auto& m : this->mesh_data)
		{
			btIndexedMesh part = {};

			part.m_vertexType = PHY_FLOAT;
			part.m_vertexBase = reinterpret_cast<const unsigned char*>(m.vertices.data());
			part.m_vertexStride = sizeof(SimpleMeshData::Vertex); // SimpleVertex
			part.m_numVertices = static_cast<int>(m.vertices.size());

			if (m.indices)
			{
				part.m_triangleIndexBase = reinterpret_cast<const unsigned char*>(m.indices->data());
				part.m_triangleIndexStride = (sizeof(SimpleMeshData::Index) * 3); // 3 indices per-triangle.
				part.m_numTriangles = static_cast<int>(m.indices->size() / 3); // 3 indices per-triangle.
				part.m_indexType = ph_index_type;
			}

			mesh_interface->addIndexedMesh(part, ph_index_type);
		}
	}

	// Empty destructor may be required for `std::unique_ptr` destruction.
	// (Complete type of `btTriangleMesh` is known in this file)
	//CollisionGeometry::~CollisionGeometry() {}

	bool CollisionGeometry::has_32bit_indices() const
	{
		return (sizeof(SimpleMeshData::Index) == 4);
	}

	std::unique_ptr<CollisionGeometry::Descriptor> CollisionGeometry::generate_mesh_interface()
	{
		//return std::make_unique<Descriptor>();
		return std::make_unique<Descriptor>(has_32bit_indices(), false);
	}
}