#pragma once

#include "types.hpp"

#include <bullet/LinearMath/btVector3.h>
#include <bullet/LinearMath/btMatrix3x3.h>
#include <bullet/LinearMath/btTransform.h>

namespace math
{
	Vector3D to_vector(const btVector3& v);
	
	Matrix to_matrix(const btTransform& t);
	Matrix to_matrix(const btMatrix3x3& bm);

	btVector3 to_bullet_vector(const Vector3D& v);
	btTransform to_bullet_matrix(const Matrix& m);
}