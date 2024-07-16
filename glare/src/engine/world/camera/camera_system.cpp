#include "camera_system.hpp"

#include "components/camera_component.hpp"

#include "commands/set_camera_command.hpp"

#include <engine/types.hpp>
#include <engine/registry.hpp>
#include <engine/transform.hpp>

#include <engine/world/world.hpp>
#include <engine/world/render/render_phase.hpp>

#include <math/math.hpp>
#include <math/conversion.hpp>

#include <cassert>

namespace engine
{
	CameraSystem::CameraSystem(World& world)
		: WorldSystem(world)
	{
		world.subscribe(*this);
	}

	Entity CameraSystem::get_active_camera() const
	{
		return this->active_camera;
	}

	void CameraSystem::set_active_camera(Entity camera)
	{
		assert((camera != null) ? static_cast<bool>(get_registry().try_get<CameraComponent>(camera)) : true);

		this->active_camera = camera;
	}

	void CameraSystem::on_subscribe(World& world)
	{
		auto& registry = get_registry();

		registry.on_construct<CameraComponent>().connect<&CameraSystem::on_camera_create>(*this);
		registry.on_destroy<CameraComponent>().connect<&CameraSystem::on_camera_destroy>(*this);

		world.register_event<SetCameraCommand, &CameraSystem::on_set_camera_command>(*this);
	}

	void CameraSystem::on_unsubscribe(World& world)
	{
		auto& registry = get_registry();

		registry.on_construct<CameraComponent>().disconnect(this);
		registry.on_destroy<CameraComponent>().disconnect(this);

		world.unregister(*this);
	}

	void CameraSystem::on_camera_create(Registry& registry, Entity entity)
	{
		if (!has_active_camera())
		{
			set_active_camera(entity);
		}
	}

	void CameraSystem::on_camera_destroy(Registry& registry, Entity entity)
	{
		if (entity == this->active_camera)
		{
			this->active_camera = null;
		}
	}

	void CameraSystem::on_set_camera_command(const SetCameraCommand& camera_command)
	{
		set_active_camera(camera_command.target);
	}

	void CameraSystem::on_update(World& world, float delta)
	{
		//update_camera_parameters(...);

		//auto& registry = get_registry();
	}

	void CameraSystem::apply_analog_input(Transform& tform, const math::Vector2D& influence, float delta) const
	{
		tform.rotate({ 0.0f, (influence.x * delta), 0.0f }, true);
		tform.rotateX((influence.y * delta), false);
	}

	void CameraSystem::update_camera_parameters(int width, int height)
	{
		auto& registry = get_registry();

		registry.view<CameraComponent>().each([&](auto entity, auto& camera_component)
		{
			// TODO: Need to look into designating viewports to cameras at the component level.
			// This would make this routine unnecessary for anything but the active camera/main viewport.
			// For now, we just check for the `dynamic_aspect_ratio` flag.
			if (camera_component.dynamic_aspect_ratio)
			{
				camera_component.update_aspect_ratio(width, height);
			}
		});
	}

	math::Vector2D CameraSystem::get_display_size_f() const
	{
		return math::Vector2D
		{
			static_cast<float>(active_camera_window_size.x),
			static_cast<float>(active_camera_window_size.y)
		};
	}

	void CameraSystem::update_active_camera_viewport(const graphics::Viewport& viewport, const math::vec2i& screen_size)
	{
		active_camera_viewport = viewport;
		active_camera_window_size = screen_size;

		update_camera_parameters(screen_size.x, screen_size.y);
	}

	math::Vector2D CameraSystem::to_normalized_device_coordinates(const math::Vector2D& device_position) const
	{
		return math::to_normalized_device_coordinates(get_display_size_f(), device_position);
	}

	math::Vector2D CameraSystem::from_normalized_device_coordinates(const math::Vector2D& normalized_position) const
	{
		return math::from_normalized_device_coordinates(get_display_size_f(), normalized_position);
	}

	math::Matrix CameraSystem::get_active_camera_matrix() const
	{
		if (active_camera == null)
		{
			return math::identity_matrix();
		}

		math::Matrix projection, view;

		RenderPhase::get_camera_matrices(world, active_camera_viewport, active_camera, projection, view);

		return (projection * view);
	}

	math::Matrix CameraSystem::get_active_camera_inverse_matrix() const
	{
		if (active_camera == null)
		{
			return math::identity_matrix();
		}

		math::Matrix projection, view;

		RenderPhase::get_camera_matrices(world, active_camera_viewport, active_camera, projection, view);

		return glm::inverse(projection * view);
	}

	DirectionalRay CameraSystem::get_ray_from_normalized_display_coordinates(const math::Vector2D& normalized_position)
	{
		auto& registry = world.get_registry();

		const auto& camera          = active_camera;
		const auto& camera_viewport = active_camera_viewport;
		const auto& camera_comp     = registry.get<engine::CameraComponent>(camera);

		auto camera_tform           = world.get_transform(camera);

		const auto projection = camera_comp.get_projection(camera_viewport);
		//const auto view = camera_tform.get_matrix();

		const auto inv_projection = glm::inverse(projection);

		//const auto inv_view = glm::inverse(view);
		const auto inv_view = camera_tform.get_matrix();

		const auto clip_space_direction = math::Vector4D(normalized_position.x, normalized_position.y, -1.0f, 1.0f);

		const auto view_local_direction = (inv_projection * clip_space_direction);

		/*
		const auto world_space_origin = math::Vector3D
		{
			inv_view
			*
			math::Vector4D
			{
				view_local_direction.x,
				view_local_direction.y,
				0.0f,
				1.0f
			}
		};
		*/

		const auto world_space_origin = camera_tform.get_position();

		const auto world_space_direction = glm::normalize
		(
			math::Vector3D
			{
				inv_view
				*
				math::Vector4D
				{
					view_local_direction.x,
					view_local_direction.y,
					-1.0f,
					0.0f // 1.0f
				}
			}
		);

		return { world_space_origin, world_space_direction };
	}

	DirectionalRay CameraSystem::get_ray_from_display_coordinates(const math::Vector2D& device_local_position)
	{
		auto normalized_position = to_normalized_device_coordinates(device_local_position);

		return get_ray_from_normalized_display_coordinates(normalized_position);
	}

	CameraComponent CameraSystem::get_active_camera_parameters() const
	{
		if (active_camera == null)
		{
			return {};
		}

		auto& registry = get_registry();

		const auto* camera_parameters = registry.try_get<CameraComponent>(active_camera);

		return *camera_parameters;
	}

	float CameraSystem::get_mouse_analog_movement_multiplier() const
	{
		constexpr float mouse_movement_multiplier = 20.0f;

		return mouse_movement_multiplier;
	}
}