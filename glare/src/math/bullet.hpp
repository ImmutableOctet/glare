#pragma once

#include "math.hpp"

#include <bullet/LinearMath/btVector3.h>
#include <bullet/LinearMath/btTransform.h>

#include <glm/gtc/type_ptr.hpp>

namespace math
{
	inline btVector3 to_bullet_vector(const Vector3D& v)
	{
		return btVector3(v.x, v.y, v.z);
	}

	inline btTransform to_bullet_matrix(const Matrix& m)
	{
		btTransform t;

		t.setFromOpenGLMatrix(glm::value_ptr(m));

		return t;
	}
}