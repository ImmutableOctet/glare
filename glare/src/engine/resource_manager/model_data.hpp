#pragma once

#include "types.hpp"

#include <math/types.hpp>

#include <vector>

namespace engine
{
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

		std::vector<ModelEntry> models; // std::small_vector<ModelEntry, ...>
	};

	using ModelData = ModelData_Raw<>;
	using StrongModelData = ModelData;
	using WeakModelData = ModelData_Raw<WeakModelRef>;
}