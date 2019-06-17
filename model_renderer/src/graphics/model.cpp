#include "model.hpp"
#include "mesh.hpp"
#include "material.hpp"

#include <algorithm>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace graphics
{
	Model::Model(const MeshData& meshes)
		: meshes(meshes) {}

	Model::Model(Model&& model)
		: Model() { swap(*this, model); }

	void swap(Model& x, Model& y)
	{
		using std::swap;

		swap(x.meshes, y.meshes);
	}

	Model Model::Load(pass_ref<Context> context, const std::string& path)
	{
		return {};
	}
}