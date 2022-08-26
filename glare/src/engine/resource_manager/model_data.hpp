#pragma once

#include <vector>
#include <engine/types.hpp>

namespace graphics
{
	class Model;
}

namespace engine
{
	using ModelRef = ref<graphics::Model>;
	using WeakModelRef = weak_ref<graphics::Model>; // const graphics::Model*

	using Models = std::vector<ModelRef>;

	// TODO: Revisit weak vs. strong model references for caching purposes. (Resource manager)
	template <typename RefType=ModelRef>
	struct ModelData_Raw
	{
		// May change this later to be the same as the `ModelLoader` class's `ModelData` type.
		struct ModelEntry
		{
			RefType model;
			math::Matrix transform;
		};

		/*
		Models models;
		std::vector<math::Matrix> matrices;
		//AnimationData animations;
		*/

		std::vector<ModelEntry> models;
	};

	using ModelData = ModelData_Raw<>;
	using StrongModelData = ModelData;
	using WeakModelData = ModelData_Raw<WeakModelRef>;
}