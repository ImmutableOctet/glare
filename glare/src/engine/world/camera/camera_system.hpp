#pragma once

#include "components/camera_component.hpp"

#include <engine/types.hpp>
#include <engine/events.hpp>

#include <engine/world/world_system.hpp>

#include <engine/world/physics/directional_ray.hpp>

#include <math/types.hpp>

#include <graphics/viewport.hpp>

namespace engine
{
	struct Transform;

	struct SetCameraCommand;
	struct OnAnalogInput;

	class CameraSystem : public WorldSystem
	{
		public:
			CameraSystem(World& world);

			// Attempts to retrieve the active primary camera.
			Entity get_active_camera() const;

			// Sets the active primary camera to `camera`.
			void set_active_camera(Entity camera);

			// Checks if there is currently an active primary camera.
			inline bool has_active_camera() const
			{
				return (get_active_camera() != null);
			}

			// Updates the active camera's viewport / screen size.
			void update_active_camera_viewport(const graphics::Viewport& viewport, const math::vec2i& screen_size);

			// Converts a regular device coordinate (i.e. pixels) to a normalized display coordinate (range of -1.0 to 1.0).
			math::Vector2D to_normalized_device_coordinates(const math::Vector2D& device_position) const;

			// Converts a normalized display coordinate (range of -1.0 to 1.0) to a regular device coordinate (i.e. pixels).
			math::Vector2D from_normalized_device_coordinates(const math::Vector2D& normalized_position) const;

			// Computes a matrix from the active camera's projection and view matrices.
			math::Matrix get_active_camera_matrix() const;

			// Computes an inverse matrix from the active camera's projection and view matrices.
			// See also: `get_active_camera_matrix`
			math::Matrix get_active_camera_inverse_matrix() const;

			// Converts `normalized_position` (normalized display coordinate; range of -1.0 to 1.0) to a world-space direction vector.
			DirectionalRay get_ray_from_normalized_display_coordinates(const math::Vector2D& normalized_position);

			// Converts `device_local_position` (Screen location in pixels) to a world-space direction vector.
			DirectionalRay get_ray_from_display_coordinates(const math::Vector2D& device_local_position);

			// Retrieves a copy of the `CameraComponent` attached to the `active_camera`.
			CameraComponent get_active_camera_parameters() const;

			// A multiplier used to increase the influence of a mouse-based analog input.
			float get_mouse_analog_movement_multiplier() const;

			void apply_analog_input(Transform& tform, const math::Vector2D& influence, float delta) const;

		protected:
			// TODO: Look into reworking/replacing this.
			void update_camera_parameters(int width, int height);

			math::Vector2D get_display_size_f() const;

			void on_subscribe(World& world) override;
			void on_unsubscribe(World& world) override;

			void on_camera_create(Registry& registry, Entity entity);
			void on_camera_destroy(Registry& registry, Entity entity);
			void on_set_camera_command(const SetCameraCommand& camera_command);

			void on_update(World& world, float delta) override;

			// The active (primary) camera.
			Entity active_camera = null;

			// Last known display-area reported to this system.
			graphics::Viewport active_camera_viewport = {};

			// Last known display-size reported to this system.
			math::vec2i active_camera_window_size = {};
	};
}