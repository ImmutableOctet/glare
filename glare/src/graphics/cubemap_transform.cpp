#include "cubemap_transform.hpp"
#include <glm/common.hpp>

namespace graphics
{
	CubeMapTransforms::CubeMapTransforms(const TFormType& projection, math::Vector point)
	{
		transforms =
		{
			(projection * glm::lookAt(point, point + math::Vector(1.0f, 0.0f, 0.0f), math::Vector(0.0f, -1.0f, 0.0f))),
			(projection * glm::lookAt(point, point + math::Vector(-1.0f, 0.0f, 0.0f), math::Vector(0.0f, -1.0f, 0.0f))),
			(projection * glm::lookAt(point, point + math::Vector(0.0f, 1.0f, 0.0f), math::Vector(0.0f, 0.0f, 1.0f))),
			(projection * glm::lookAt(point, point + math::Vector(0.0f, -1.0f, 0.0f), math::Vector(0.0f, 0.0f, -1.0f))),
			(projection * glm::lookAt(point, point + math::Vector(0.0f, 0.0f, 1.0f), math::Vector(0.0f, -1.0f, 0.0f))),
			(projection * glm::lookAt(point, point + math::Vector(0.0f, 0.0f, -1.0f), math::Vector(0.0f, -1.0f, 0.0f)))
		};
	}

	CubeMapTransforms::CubeMapTransforms(TFormData transforms)
		: transforms(transforms) {}
}