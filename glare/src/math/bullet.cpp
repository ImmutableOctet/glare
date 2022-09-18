#pragma once

#include "bullet.hpp"

#include <glm/gtc/type_ptr.hpp>

namespace math
{
	Vector3D to_vector(const btVector3& v)
	{
		return { v.getX(), v.getY(), v.getZ() };
	}

	Matrix to_matrix(const btTransform& t)
	{
		Matrix m;

		t.getOpenGLMatrix(glm::value_ptr(m));

		return m;
	}

	Matrix to_matrix(const btMatrix3x3& bm)
	{
		//float matrix_data[3 * 4];
		//bm.getOpenGLSubMatrix(matrix_data);

		Matrix3x3 m;

		auto* matrix_data = glm::value_ptr(m);

		const auto& r1 = bm[0];

		matrix_data[0] = r1.x();
		matrix_data[1] = r1.y();
		matrix_data[2] = r1.z();

		const auto& r2 = bm[1];

		matrix_data[3] = r2.x();
		matrix_data[4] = r2.y();
		matrix_data[5] = r2.z();
		
		const auto& r3 = bm[2];

		matrix_data[6] = r3.x();
		matrix_data[7] = r3.y();
		matrix_data[8] = r3.z();

		return m;
	}

	btVector3 to_bullet_vector(const Vector3D& v)
	{
		return btVector3(v.x, v.y, v.z);
	}

	btTransform to_bullet_matrix(const Matrix& m)
	{
		btTransform t;

		t.setFromOpenGLMatrix(glm::value_ptr(m));

		return t;
	}
}