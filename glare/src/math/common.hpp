#pragma once

#include "types.hpp"

#include <cmath>

namespace math
{
	// Imported GLM functions:
	using glm::dot;
	using glm::cross;
	using glm::normalize;
	using glm::length;
	using glm::inverse;
	using glm::transpose;
	using glm::translate;
	using glm::rotate;
	using glm::scale;

	// Standard math functions:
	using std::abs;
	using std::sqrt;

	using std::sin;
	using std::cos;
	using std::tan;
	using std::asin;
	using std::acos;
	using std::atan;
	using std::atan2;
	
	using std::sinh;
	using std::cosh;
	using std::tanh;
	using std::asinh;
	using std::acosh;
	using std::atanh;

	template <typename matrix_type>
	constexpr matrix_type identity()
	{
		return matrix_type(1.0f); // glm::identity<matrix_type>();
	}

	// Same as calling `identity<Matrix>()`.
	inline constexpr Matrix identity_matrix()
	{
		return identity<Matrix>();
	}

	// Returns 1 for values greater than zero,
	// -1 for values less than zero,
	// and 0 for exactly 0.
	template <typename T, typename ResultType=int>
	ResultType sign(T value)
	{
		constexpr auto zero = T(0);

		return ResultType((zero < value) - (value < zero));
	}

	// Returns the `sign` from the result of: (x-y)
	//
	// If `x` is less than `y` this returns -1,
	// if `x` is greater than `y` this returns 1,
	// if `x` and `y` are equal this returns 0.
	template <typename T, typename ResultType=int>
	ResultType sign(const T& x, const T& y)
	{
		return sign<T, ResultType>((x - y));
	}

	// Forces `value` to be positive.
	template <typename T>
	T positive(const T& value)
	{
		return (value * sign(value));
	}

	// Forces `value` to be negative.
	template <typename T>
	T negative(const T& value)
	{
		//return (value * -sign(value));

		return -positive(value);
	}

	// Inverts the sign of `value`.
	// (Equivalent to unary `-` operator)
	template <typename T>
	T negate(const T& value)
	{
		return -value;
	}

	template <typename T>
	T sq(const T& x)
	{
		return (x * x);
	}

	template <typename T, typename MinType=T, typename MaxType=T>
	T clamp(const T& value, const MinType& min_value, const MaxType& max_value)
	{
		if (value <= min_value)
		{
			return min_value;
		}

		if (value >= max_value)
		{
			return max_value;
		}

		return value;
	}

	inline Vector2D abs(const Vector2D& v)
	{
		return { abs(v.x), abs(v.y) };
	}

	inline Vector3D abs(const Vector3D& v)
	{
		return { abs(v.x), abs(v.y), abs(v.z) };
	}

	inline Vector4D abs(const Vector4D& v)
	{
		return { abs(v.x), abs(v.y), abs(v.z), abs(v.w) };
	}
}