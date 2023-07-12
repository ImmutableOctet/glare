#include "collision_events.hpp"

#include <math/common.hpp>
#include <math/surface.hpp>
#include <math/rotation.hpp>

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
		return math::length(impact_velocity);
	}

	math::Vector OnSurfaceContact::direction() const
	{
		return math::normalize(impact_velocity);
	}

	math::Vector OnSurfaceContact::penetration_vector() const
	{
		return (direction() * penetration());
	}

	float OnSurfaceContact::dot_product() const
	{
		return math::dot(collision.normal, direction());
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
		return math::dot(direction(), surface_forward);
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

	math::OrthogonalVectors OnSurfaceContact::surface_orientation_vectors(const math::Vector& forward) const
	{
		auto right = math::cross(collision.normal, forward); // math::normalize(...)

		return { right, collision.normal, forward };
	}

	math::Vector OnSurfaceContact::forward(const math::Vector& forward) const
	{
		auto orientation_vectors = surface_orientation_vectors(forward);

		// Retrieve only the `forward` vector.
		return std::get<2>(orientation_vectors);
	}

	math::RotationMatrix OnSurfaceContact::alignment(const math::Vector& forward) const
	{
		return math::inverse(math::rotation_from_orthogonal(surface_orientation_vectors(-forward)));
	}

	math::Quaternion OnSurfaceContact::alignment_q(const math::Vector& forward) const
	{
		return math::inverse(math::quaternion_from_orthogonal(surface_orientation_vectors(-forward)));
	}

	float OnSurfaceContact::vertical_sign() const
	{
		return math::sign<float, float>(collision.position.y, collision.a_position.y);
	}
}