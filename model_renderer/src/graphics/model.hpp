#pragma once

#include <string>
#include <vector>
#include <utility>

#include <util/memory.hpp>

#include "mesh.hpp"
#include "material.hpp"

namespace graphics
{
	class Context;
	class Canvas;
	//class Material;
	//class Mesh;

	class Model
	{
		public:
			using MeshPair = std::pair<ref<Mesh>, ref<Material>>;
			using MeshData = std::vector<MeshPair>;
		private:
			MeshData meshes;
		protected:
			Model(Context& ctx, const MeshData& meshes);
		public:
			Model Load(const std::string& path);
	};
}