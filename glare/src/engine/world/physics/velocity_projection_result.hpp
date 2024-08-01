#pragma once

#include <math/types.hpp>

namespace engine
{
	struct VelocityProjectionResult
	{
		math::Vector projected_velocity = {};

		float surface_angle            = {};

		float ground_alignment         = {};
		float inverse_ground_alignment = {};

		float wall_alignment         = {};
		float inverse_wall_alignment = {};

		bool is_ground_surface : 1 = false;
		bool is_wall_surface   : 1 = false;

		inline bool is_valid() const
		{
			return ((is_ground_surface) || (is_wall_surface));
		}

		inline VelocityProjectionResult& operator<<(const math::Vector& velocity)
		{
			projected_velocity += velocity;

			return *this;
		}

		inline VelocityProjectionResult& operator<<(const VelocityProjectionResult& result)
		{
			return ((*this) << result.projected_velocity);
		}

		inline explicit operator bool() const
		{
			return is_valid();
		}
	};
}