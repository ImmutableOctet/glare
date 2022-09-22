#include "collision_events.hpp"

#include <math/math.hpp>

#include <tuple>

namespace engine
{
	// OnSurfaceContact:

	float OnSurfaceContact::penetration() const
	{
		return collision.penetration;
	}

	float OnSurfaceContact::contact_speed() const
	{
		return glm::length(impact_velocity);
	}

	math::Vector OnSurfaceContact::direction() const
	{
		return glm::normalize(impact_velocity);
	}

	math::Vector OnSurfaceContact::penetration_vector() const
	{
		return (direction() * penetration());
	}

	float OnSurfaceContact::dot_product() const
	{
		return glm::dot(collision.normal, direction());
	}

	float OnSurfaceContact::force_applied() const
	{
		return (contact_speed() - penetration());
	}

	// Ratio of `force_applied` to the `penetration` observed.
	float OnSurfaceContact::residual() const
	{
		return (force_applied() / penetration());
	}

	float OnSurfaceContact::floor_product() const
	{
		return (dot_product() * vertical_sign()); // math::abs(dot_product())
	}

	float OnSurfaceContact::floor() const
	{
		auto fp = floor_product();

		if (fp > 0.0f)
		{
			return fp;
		}

		return 0.0f;
	}

	float OnSurfaceContact::ceiling_no_convert() const
	{
		auto fp = floor_product();

		if (fp < 0.0f)
		{
			return fp;
		}

		return 0.0f;
	}

	float OnSurfaceContact::ceiling() const
	{
		// Flip from negative values to positive values.
		return -ceiling_no_convert();
	}

	float OnSurfaceContact::slope(const math::Vector& surface_forward) const
	{
		return glm::dot(direction(), surface_forward);
	}

	float OnSurfaceContact::slope() const
	{
		// We call the `math` implementation here, but this is effectively the same
		// as using `slope_orientation_vectors` and `slope` with a surface-forward vector.
		return math::get_surface_slope(collision.normal, direction());
	}

	float OnSurfaceContact::slope(const math::OrthogonalVectors& orientation) const
	{
		return slope(std::get<1>(orientation));
	}

	math::OrthogonalVectors OnSurfaceContact::surface_orientation_vectors(const math::Vector& adjacent) const
	{
		auto forward = math::get_surface_forward(collision.normal, adjacent);

		return { adjacent, collision.normal, forward };
	}

	float OnSurfaceContact::vertical_sign() const
	{
		return math::sign<float, float>(collision.position.y, collision.a_position.y);
	}
}