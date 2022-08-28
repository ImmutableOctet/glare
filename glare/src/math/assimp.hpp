#pragma once

#include <assimp/vector3.h>
#include <assimp/matrix4x4.h>
#include <assimp/Quaternion.h>

#include "math.hpp"

namespace math
{
	inline Quaternion to_quat(const aiQuaternion& q)
	{
		return Quaternion(q.w, q.x, q.y, q.z);
	}

	inline Quaternion to_quat_flipped(const aiQuaternion& q)
	{
		//return Quaternion(-q.w, q.z, q.y, q.x);
		//return Quaternion(q.w, q.x, -q.y, q.z);
		//return Quaternion(q.w, q.z, q.y, q.x);
		//return Quaternion(q.w, -q.z, q.y, -q.x);

		//return Quaternion(q.w, -q.x, q.y, -q.z);
		return Quaternion(q.w, -q.x, -q.y, -q.z);
	}

	inline Vector3D to_vector(const aiVector3D& v)
	{
		return { v.x, v.y, v.z };
	}

	inline Matrix to_matrix(const aiMatrix4x4& m)
	{
		Matrix out;
		
		// Fields a, b, c, and d from Assimp effectively need to be transposed/flipped for GLM:
		out[0][0] = m.a1; out[1][0] = m.a2; out[2][0] = m.a3; out[3][0] = m.a4;
		out[0][1] = m.b1; out[1][1] = m.b2; out[2][1] = m.b3; out[3][1] = m.b4;
		out[0][2] = m.c1; out[1][2] = m.c2; out[2][2] = m.c3; out[3][2] = m.c4;
		out[0][3] = m.d1; out[1][3] = m.d2; out[2][3] = m.d3; out[3][3] = m.d4;

		return out;
	}
}