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

	struct ModelData
	{
		// May change this later to be the same as the `ModelLoader` class's `ModelData` type.
		struct ModelEntry
		{
			ModelRef model;
			math::Matrix transform;
		};

		/*
		Models models;
		std::vector<math::Matrix> matrices;
		//AnimationData animations;
		*/

		std::vector<ModelEntry> models;
	};
}