#include "reflection.hpp"

#include "camera_system.hpp"

#include "components/camera_component.hpp"
#include "components/camera_orbit_component.hpp"
#include "components/camera_control_component.hpp"
#include "components/camera_freelook_component.hpp"

#include "commands/set_camera_command.hpp"

#include "events.hpp"

#include <graphics/viewport.hpp>

namespace engine
{
	template <>
	void reflect<CameraComponent>()
	{
		auto camera_component = engine_meta_type<CameraComponent>()
			//.data<&CameraComponent::fov>("fov"_hs)
			.data<&CameraComponent::set_vertical_fov, &CameraComponent::get_vertical_fov>("fov"_hs)

			.data<&CameraComponent::near_plane>("near_plane"_hs)
			.data<&CameraComponent::far_plane>("far_plane"_hs)
			.data<&CameraComponent::aspect_ratio>("aspect_ratio"_hs)
			.data<&CameraComponent::projection_mode>("projection_mode"_hs)

			.data<&CameraComponent::set_free_rotation, &CameraComponent::get_free_rotation>("free_rotation"_hs)
			.data<&CameraComponent::set_dynamic_aspect_ratio, &CameraComponent::get_dynamic_aspect_ratio>("dynamic_aspect_ratio"_hs)

			//.data<&CameraComponent::set_horizontal_fov, &CameraComponent::get_horizontal_fov>("h_fov"_hs)
			//.data<&CameraComponent::set_vertical_fov, &CameraComponent::get_vertical_fov>("v_fov"_hs)

			.func<&CameraComponent::get_orthographic_projection>("get_orthographic_projection"_hs)
			.func<&CameraComponent::get_perspective_projection>("get_perspective_projection"_hs)
			.func<&CameraComponent::get_projection>("get_projection"_hs)

			.ctor
			<
				decltype(CameraComponent::fov),
				
				decltype(CameraComponent::near_plane),
				decltype(CameraComponent::far_plane),
				decltype(CameraComponent::aspect_ratio),
				decltype(CameraComponent::projection_mode),
				
				bool, bool
			>()
		;
	}

	template <>
	void reflect<CameraOrbitComponent>()
	{
		engine_meta_type<CameraOrbitComponent>()
			.data<&CameraOrbitComponent::movement_speed>("movement_speed"_hs)
			.data<&CameraOrbitComponent::focus_offset>("focus_offset"_hs)
			.data<&CameraOrbitComponent::distance>("distance"_hs)
			.data<&CameraOrbitComponent::min_distance>("min_distance"_hs)
			.data<&CameraOrbitComponent::max_distance>("max_distance"_hs)
			.data<&CameraOrbitComponent::target>("target"_hs)
		;
	}

	template <>
	void reflect<CameraFreeLookComponent>()
	{
		engine_meta_type<CameraFreeLookComponent>()
			.data<&CameraFreeLookComponent::movement_speed>("movement_speed"_hs)
		;
	}

	template <>
	void reflect<CameraControlComponent>()
	{
		engine_meta_type<CameraControlComponent>()
			.data<&CameraControlComponent::look>("look"_hs)
			.data<&CameraControlComponent::zoom>("zoom"_hs)
			.data<&CameraControlComponent::move>("move"_hs)
		;
	}

	template <>
	void reflect<OnRelativeAnalogInput>()
	{
		engine_meta_type<OnRelativeAnalogInput>()
			.base<OnAnalogInput>()
			.data<&OnRelativeAnalogInput::direction>("direction"_hs)
			.data<&OnRelativeAnalogInput::source>("source"_hs)

			// Alias to `source` field.
			.data<&OnRelativeAnalogInput::source>("entity"_hs)
		;
	}

	template <>
	void reflect<SetCameraCommand>()
	{
		engine_command_type<SetCameraCommand>();
	}

	template <>
	void reflect<CameraSystem>()
	{
		auto type = engine_system_type<CameraSystem>()
			.data<&CameraSystem::set_active_camera, &CameraSystem::get_active_camera>("active_camera"_hs)
			.data<nullptr, &CameraSystem::get_active_camera_parameters>("active_camera_parameters"_hs)
			.data<nullptr, &CameraSystem::get_active_camera_matrix>("active_camera_matrix"_hs)
			.data<nullptr, &CameraSystem::get_active_camera_inverse_matrix>("active_camera_inverse_matrix"_hs)

			.func<&CameraSystem::to_normalized_device_coordinates>("to_normalized_device_coordinates"_hs)
			.func<&CameraSystem::from_normalized_device_coordinates>("from_normalized_device_coordinates"_hs)

			.func<&CameraSystem::get_ray_from_normalized_display_coordinates>("get_ray_from_normalized_display_coordinates"_hs)
			.func<&CameraSystem::get_ray_from_display_coordinates>("get_ray_from_display_coordinates"_hs)
		;

		//reflect<graphics::Viewport>(); // graphics::PointRect

		reflect<CameraComponent>();
		reflect<CameraOrbitComponent>();
		reflect<CameraFreeLookComponent>();
		reflect<CameraControlComponent>();

		reflect<OnRelativeAnalogInput>();

		reflect<SetCameraCommand>();
	}
}