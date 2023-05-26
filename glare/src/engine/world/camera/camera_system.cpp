#include "camera_system.hpp"

#include "components/camera_component.hpp"
#include "components/camera_control_component.hpp"
#include "components/camera_freelook_component.hpp"

#include "commands/set_camera_command.hpp"

#include "events.hpp"

#include <engine/types.hpp>
#include <engine/registry.hpp>
#include <engine/transform.hpp>

#include <engine/components/player_target_component.hpp>

#include <engine/world/world.hpp>
#include <engine/world/render/render_phase.hpp>

#include <engine/world/motion/components/focus_component.hpp>
#include <engine/world/motion/components/orbit_component.hpp>

#include <engine/input/events.hpp>

#include <math/math.hpp>
#include <math/conversion.hpp>

#include <cassert>

// Debugging related:
#include <glm/gtc/quaternion.hpp>

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
		world.register_event<OnAnalogInput, &CameraSystem::on_analog_input>(*this);
	}

	void CameraSystem::on_unsubscribe(World& world)
	{
		auto& registry = get_registry();

		registry.on_construct<CameraComponent>().disconnect(*this);
		registry.on_destroy<CameraComponent>().disconnect(*this);

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

	void CameraSystem::on_analog_input(const OnAnalogInput& analog_input)
	{
		const auto& delta_time = world.get_delta_time();
		const auto delta = static_cast<float>(delta_time);

		generate_camera_relative_input(analog_input);

		handle_orbiting_camera_input(analog_input, delta);
		handle_first_person_camera_input(analog_input, delta);
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

	void CameraSystem::handle_orbiting_camera_input(const OnAnalogInput& analog_input, float delta)
	{
		auto& registry = get_registry();

		registry.view<RelationshipComponent, TransformComponent, OrbitComponent, CameraControlComponent, PlayerTargetComponent>().each
		(
			[&](const auto entity, const RelationshipComponent& relationship, TransformComponent& tform_comp, OrbitComponent& orbit, const CameraControlComponent& camera_control, const PlayerTargetComponent& player_target)
			{
				if ((analog_input.player_index() == player_target.player_index))
				{
					if (analog_input.analog == camera_control.look)
					{
						auto target = relationship.get_parent();
						auto focus_offset = math::Vector {};

						if (const auto* focus = registry.try_get<FocusComponent>(entity))
						{
							if (focus->target != null)
							{
								target = focus->target;
							}

							focus_offset = focus->focus_offset;
						}

						if (target == null)
						{
							return;
						}

						auto entity_tform = Transform(registry, entity, relationship, tform_comp);
						auto target_tform = Transform(registry, target);

						const auto entity_initial_position = entity_tform.get_position();
						const auto target_position = (target_tform.get_position() + focus_offset);

						// TODO: Move into update event for `OrbitComponent`.
						orbit.distance = math::clamp(orbit.distance, orbit.min_distance, orbit.max_distance);

						const auto& input = analog_input.value;

						const auto movement_speed = (analog_input.is_mouse_event())
							? (orbit.movement_speed * mouse_movement_multiplier)
							: (orbit.movement_speed)
						;

						//entity_tform.look_at(target_position);

						apply_analog_input(entity_tform, (math::Vector2D { movement_speed } * input), delta);

						entity_tform.set_position(target_position);
						entity_tform.move({ 0.0f, 0.0f, orbit.distance }, true);
					}
					else if (analog_input.analog == camera_control.zoom)
					{
						orbit.distance += (analog_input.value.y * orbit.movement_speed.z);
					}
				}
			}
		);
	}

	void CameraSystem::handle_first_person_camera_input(const OnAnalogInput& analog_input, float delta)
	{
		auto& registry = get_registry();

		registry.view<RelationshipComponent, TransformComponent, CameraFreeLookComponent, CameraControlComponent, PlayerTargetComponent>().each
		(
			[&](const auto entity, const RelationshipComponent& relationship, TransformComponent& tform_comp, CameraFreeLookComponent& free_look, const CameraControlComponent& camera_control, const PlayerTargetComponent& player_target)
			{
				if ((analog_input.player_index() == player_target.player_index))
				{
					if (analog_input.analog == camera_control.look)
					{
						const auto& input = analog_input.value;

						const auto movement_speed = (analog_input.is_mouse_event())
							? (free_look.movement_speed * mouse_movement_multiplier)
							: (free_look.movement_speed)
						;

						auto tform = Transform(registry, entity, relationship, tform_comp);

						apply_analog_input(tform, (movement_speed * input), delta);
					}
				}
			}
		);
	}

	void CameraSystem::apply_analog_input(Transform& tform, const math::Vector2D& influence, float delta)
	{
		tform.rotate({ 0.0f, (influence.x * delta), 0.0f }, true);
		tform.rotateX((influence.y * delta), false);
	}

	void CameraSystem::generate_camera_relative_input(const OnAnalogInput& analog_input)
	{
		auto& registry = get_registry();

		registry.view<RelationshipComponent, TransformComponent, CameraComponent, PlayerTargetComponent>().each
		(
			[&](const auto entity, const RelationshipComponent& relationship, TransformComponent& tform_comp, const CameraComponent& camera_comp, const PlayerTargetComponent& player_target)
			{
				if (analog_input.player_index() == player_target.player_index) // && (analog_input.value.length() > 0.0f)
				{
					const auto& input_direction = analog_input.value;

					auto tform = Transform(registry, entity, relationship, tform_comp);

					const auto basis = tform.get_basis_q();

					// NOTE: Initial normalization may not be needed in the case of `Analog::Movement`,
					// since we re-normalize after removing the `y` elements.
					auto camera_forward = glm::normalize(basis * math::Vector3D { 0.0f, 0.0f, -1.0f });
					auto camera_right = glm::normalize(basis * math::Vector3D { 1.0f, 0.0f, 0.0f });

					if (analog_input.analog == Analog::Movement)
					{
						// Remove vertical movement, then re-normalize the 'xz' vectors:
						camera_forward.y = 0.0f;
						camera_right.y = 0.0f;

						// NOTE: If we don't normalize after zeroing `y`, the portion of the length distributed to `y`
						// would be missing, thus giving weaker 'xz' values at steep camera angles.
						camera_forward = glm::normalize(camera_forward);
						camera_right = glm::normalize(camera_right);
					}

					const auto relative_forward = (input_direction.y * camera_forward);
					const auto relative_right   = (input_direction.x * camera_right);

					const auto world_space_direction_3d = (relative_forward + relative_right);
					const auto world_space_direction_2d = math::Vector2D { world_space_direction_3d.x, world_space_direction_3d.z };

					auto relative_analog_input = OnRelativeAnalogInput { analog_input };

					relative_analog_input.value = world_space_direction_2d;
					relative_analog_input.angle = math::direction_to_angle_90_degrees(world_space_direction_2d);
					relative_analog_input.direction = world_space_direction_3d;

					relative_analog_input.source = entity;

					world.event<OnRelativeAnalogInput>(std::move(relative_analog_input));
				}
			}
		);
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
}