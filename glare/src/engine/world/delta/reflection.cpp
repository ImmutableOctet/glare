#include "reflection.hpp"

#include "delta_system.hpp"

#include <engine/delta_time.hpp>

#include <app/types.hpp>
#include <math/types.hpp>

#include <string>

namespace engine
{
	namespace impl
	{
		std::string delta_as_string_impl(const DeltaSystem& delta_system)
		{
			return std::to_string(delta_system.get_delta());
		}
	}

	// TODO: Look into migrating this to the main `engine:reflection` submodule.
	template <>
	void reflect<DeltaTime>()
	{
		custom_meta_type<DeltaTime>("DeltaTime"_hs)
			.data<nullptr, &DeltaTime::get_delta>("delta"_hs)
			.data<nullptr, static_cast<DeltaTime::Scalar (DeltaTime::*)() const>(&DeltaTime::get_inv_delta)>("inv_delta"_hs)
			.data<nullptr, &DeltaTime::get_interval>("interval"_hs)
			.data<&DeltaTime::set_rate, &DeltaTime::get_rate>("rate"_hs)
			.data<nullptr, &DeltaTime::current_frame_time>("current_frame_time"_hs)
			.data<nullptr, &DeltaTime::size>("size"_hs)

			.func<&DeltaTime::calculate_interval>("calculate_interval"_hs)
			.func<static_cast<DeltaTime::Scalar(*)(DeltaTime::Scalar)>(&DeltaTime::get_inv_delta)>("get_inv_delta"_hs)

			.func<&DeltaTime::per_frame<float>>("per_frame"_hs)
			.func<&DeltaTime::per_frame<math::Vector2D>>("per_frame"_hs)
			.func<&DeltaTime::per_frame<math::Vector3D>>("per_frame"_hs)
			.func<&DeltaTime::per_frame<math::Vector4D>>("per_frame"_hs)

			.func<&DeltaTime::reset_log>("reset_log"_hs)

			.func<static_cast<void (DeltaTime::*)(DeltaTime::Time, bool)>(&DeltaTime::reset)>("reset"_hs)
			.func<static_cast<void (DeltaTime::*)(DeltaTime::Rate, DeltaTime::Time, bool)>(&DeltaTime::reset)>("reset"_hs)

			.func<&DeltaTime::update>("update"_hs)

			.ctor<DeltaTime::Rate, DeltaTime::Time, DeltaTime::Scalar>()
			.ctor<DeltaTime::Rate, DeltaTime::Time>()
			.ctor<DeltaTime::Rate>()
		;
	}

	template <>
	void reflect<DeltaSystem>()
	{
		auto type = engine_system_type<DeltaSystem>()
			.conv<&DeltaSystem::get_delta>()
			.conv<impl::delta_as_string_impl>()

			.data<nullptr, static_cast<const DeltaTime& (DeltaSystem::*)() const>(&DeltaSystem::get_delta_time)>("delta_time"_hs)
			.data<nullptr, &DeltaSystem::get_delta>("delta"_hs)
		;

		reflect<DeltaTime>();
	}
}