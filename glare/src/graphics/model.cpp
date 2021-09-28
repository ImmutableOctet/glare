#include "model.hpp"
#include "mesh.hpp"
#include "material.hpp"

#include <math/math.hpp>
#include <engine/resource_manager/resource_manager.hpp>

#include <algorithm>
#include <iostream>
#include <filesystem>
#include <utility>

#include <assimp/material.h>

namespace graphics
{
	Model::Model(Meshes&& meshes, VertexWinding vertex_winding) noexcept
		: meshes(std::move(meshes)), vertex_winding(vertex_winding) {}

	Model::Model(Model&& model) noexcept
		: Model() { swap(*this, model); }

	void swap(Model& x, Model& y)
	{
		using std::swap;

		swap(x.meshes, y.meshes);
		swap(x.vertex_winding, y.vertex_winding);
	}

	const char* Model::get_texture_class_variable_raw(TextureClass type)
	{
		switch (type)
		{
			case TextureClass::Diffuse:
				return "diffuse";
			case TextureClass::Specular:
				return "specular";
			//case TextureClass::Ambient:
			//	return "ambient";
			case TextureClass::Emissive:
				return "emissive";
			case TextureClass::Height:
				return "height_map";
			case TextureClass::Normals:
				return "normal_map";
			case TextureClass::Shininess:
				return "shininess";
			case TextureClass::Opacity:
				return "opacity";
			case TextureClass::Displacement:
				return "displacement";
			case TextureClass::Lightmap:
				return "light_map";
			case TextureClass::Reflection:
				return "reflection";
			case TextureClass::BaseColor:
				return "base_color";
			case TextureClass::NormalCamera:
				return "normal_camera";
			case TextureClass::EmissionColor:
				return "emission_color";
			case TextureClass::Metalness:
				return "metalness";
			case TextureClass::DiffuseRoughness:
				return "diffuse_roughness";
			case TextureClass::AmbientOcclusion:
				return "ambient_occlusion";
			case TextureClass::Unknown:
				return "other";
		}

		return {};
	}
	
	Model::CollisionGeometry::CollisionGeometry(Container&& mesh_data)
		: mesh_interface(std::make_unique<Descriptor>()), mesh_data(std::move(mesh_data))
	{
		constexpr auto ph_index_type = ((sizeof(SimpleMeshData::Index) == 4) ? PHY_INTEGER : PHY_SHORT);

		for (const auto& m : this->mesh_data)
		{
			btIndexedMesh part = {};

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
}