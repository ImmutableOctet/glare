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
			Model(const MeshData& meshes);
		public:
			static Model Load(pass_ref<Context> context, const std::string& path);

			friend void swap(Model& x, Model& y);

			Model() = default;

			// TODO: Handle copies.
			Model(const Model&) = delete;

			Model(Model&& model);

			inline Model& operator=(Model model)
			{
				swap(*this, model);

				return *this;
			}

			inline void draw() {}
	};
}