#pragma once

#include "reflection.hpp"

#include "world.hpp"
#include "world_properties.hpp"
#include "graphics_entity.hpp"

#include "components/camera_component.hpp"
#include "components/light_component.hpp"

#include <engine/components/model_component.hpp>
#include <graphics/model.hpp>
#include <graphics/shader.hpp>

namespace engine
{
	struct ModelComponent;

	namespace world_impl
	{
		template <typename T>
		void extend() {}

		template <>
		inline void extend<ModelComponent>()
		{
			entt::meta<ModelComponent>()
				.func<&attach_model>("attach_model"_hs)
				.func<&create_model>("create_model"_hs)
				.func<&load_model>("load_model"_hs)
				.func<&load_model_attachment>("load_model_attachment"_hs)
				.func<&create_cube>("create_cube"_hs)
			;
		}

		void reflect_extensions()
		{
			extend<ModelComponent>();
		}
	}

	template <>
	void reflect<CameraComponent>()
	{
		engine_meta_type<CameraComponent>()
			.data<&CameraComponent::fov>("fov"_hs)
			.data<&CameraComponent::aspect_ratio>("aspect_ratio"_hs)
			.data<&CameraComponent::near_plane>("near_plane"_hs)
			.data<&CameraComponent::far_plane>("far_plane"_hs)
			.data<&CameraComponent::projection_mode>("projection_mode"_hs)

			// Getters/setters:
			.data<&CameraComponent::set_free_rotation, &CameraComponent::get_free_rotation>("free_rotation"_hs)
			.data<&CameraComponent::set_dynamic_aspect_ratio, &CameraComponent::get_dynamic_aspect_ratio>("dynamic_aspect_ratio"_hs)

			.ctor
			<
				decltype(CameraComponent::fov),
				decltype(CameraComponent::aspect_ratio),
				decltype(CameraComponent::near_plane),
				decltype(CameraComponent::far_plane),
				decltype(CameraComponent::projection_mode),
				
				bool, bool
			>()
			
			.ctor
			<
				decltype(CameraComponent::fov),
				decltype(CameraComponent::aspect_ratio),
				decltype(CameraComponent::near_plane),
				decltype(CameraComponent::far_plane),
				decltype(CameraComponent::projection_mode)
			>()
		;
	}

	template <>
	void reflect<LightProperties>()
	{
		engine_meta_type<LightProperties>()
			.data<&LightProperties::ambient>("ambient"_hs)
			.data<&LightProperties::diffuse>("diffuse"_hs)
			.data<&LightProperties::specular>("specular"_hs)
			.ctor
			<
				decltype(LightProperties::ambient),
				decltype(LightProperties::diffuse),
				decltype(LightProperties::specular)
			>()
		;
	}

	template <>
	void reflect<DirectionalLightComponent>()
	{
		engine_meta_type<DirectionalLightComponent>()
			.data<&DirectionalLightComponent::set_use_position, &DirectionalLightComponent::get_use_position>("use_position"_hs)
		;
	}

	template <>
	void reflect<SpotLightComponent>()
	{
		engine_meta_type<SpotLightComponent>()
			.data<&SpotLightComponent::cutoff>("cutoff"_hs)
			.data<&SpotLightComponent::outer_cutoff>("outer_cutoff"_hs)
			.data<&SpotLightComponent::constant>("constant"_hs)
			.data<&SpotLightComponent::linear>("linear"_hs)
			.data<&SpotLightComponent::quadratic>("quadratic"_hs)

			.ctor
			<
				decltype(SpotLightComponent::cutoff),
				decltype(SpotLightComponent::outer_cutoff),
				decltype(SpotLightComponent::constant),
				decltype(SpotLightComponent::linear),
				decltype(SpotLightComponent::quadratic)
			>()

			.ctor
			<
				decltype(SpotLightComponent::cutoff),
				decltype(SpotLightComponent::outer_cutoff)
			>()
		;
	}

	template <>
	void reflect<PointLightComponent>()
	{
		engine_meta_type<PointLightComponent>()
			.data<&PointLightComponent::linear>("linear"_hs)
			.data<&PointLightComponent::quadratic>("quadratic"_hs)

			.ctor
			<
				decltype(PointLightComponent::linear),
				decltype(PointLightComponent::quadratic)
			>()

			//.func<static_cast<float(*)(float, float)>(&PointLightComponent::get_radius)>("get_radius"_hs)
			.func<&PointLightComponent::get_radius_f>("get_radius"_hs)
		;
	}

	template <>
	void reflect<LightComponent>()
	{
		// TODO: Migrate this to `engine`'s `reflect_primitives` subroutine.
		reflect<LightProperties>();

		engine_meta_type<LightComponent>()
			.data<&LightComponent::type>("type"_hs)
			.data<&LightComponent::properties>("properties"_hs)
			.func<&LightComponent::resolve_light_mode>("resolve_light_mode"_hs)
			.ctor<decltype(LightComponent::type), decltype(LightComponent::properties)>()
		;

		reflect<DirectionalLightComponent>();
		reflect<SpotLightComponent>();
		reflect<PointLightComponent>();
	}

	template <>
	void reflect<WorldProperties>()
	{
		engine_meta_type<WorldProperties>()
			.data<&WorldProperties::ambient_light>("ambient_light"_hs)
			.data<&WorldProperties::gravity>("gravity"_hs)

			.ctor
			<
				decltype(WorldProperties::ambient_light),
				decltype(WorldProperties::gravity)
			>()
		;
	}

	// TODO: Implement a formal reflected interface for `World`.
	template <>
	void reflect<World>()
	{
		reflect<CameraComponent>();
		reflect<LightComponent>();  // NOTE: Handles several lighting-related types.
		reflect<WorldProperties>();

		reflect_behaviors();

		reflect<AnimationSystem>();
		reflect<PhysicsSystem>();
		reflect<MotionSystem>();

		// ...

		// Extensions:
		world_impl::reflect_extensions();
	}
}