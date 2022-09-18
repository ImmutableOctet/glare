#pragma once

#include "types.hpp"

#include <math/types.hpp>

#include <array>

namespace graphics
{
	// Container for 6 view-projection mattrices representing different faces of a cube-map.
	struct CubeMapTransforms
	{
		static constexpr std::size_t FACES = 6;

		using TFormType = math::Matrix4x4;
		using TFormData = std::array<TFormType, FACES>;

		CubeMapTransforms(const TFormType& projection, math::Vector point);
		CubeMapTransforms(TFormData transforms);

		TFormData transforms;

		inline constexpr std::size_t size() const
		{
			//return FACES;
			return transforms.size();
		}
	};
}