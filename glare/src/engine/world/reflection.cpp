#pragma once

#include "reflection.hpp"

#include "world.hpp"
#include "world_properties.hpp"
#include "graphics_entity.hpp"

#include "components/camera_component.hpp"
#include "components/light_component.hpp"
#include "components/directional_light_component.hpp"
#include "components/directional_light_shadow_component.hpp"
#include "components/point_light_component.hpp"
#include "components/point_light_shadow_component.hpp"
#include "components/spot_light_component.hpp"

#include <engine/components/model_component.hpp>
#include <graphics/model.hpp>
#include <graphics/shader.hpp>

namespace engine
{
	struct ModelComponent;

	namespace world_impl
	{
		template <typename T>
		static void extend() {}

		template <>
		static void extend<ModelComponent>()
		{
			entt::meta<ModelComponent>()
				.func<&attach_model>("attach_model"_hs)
				.func<&create_model>("create_model"_hs)
				.func<&load_model>("load_model"_hs)
				.func<&load_model_attachment>("load_model_attachment"_hs)
				.func<&create_cube>("create_cube"_hs)
			;
		}

		static void reflect_extensions()
		{
			extend<ModelComponent>();
		}

		template <typename ShadowComponentType>
		static void reflect_shadow_component()
		{
			engine_meta_type<ShadowComponentType>()
				.data<&ShadowComponentType::shadow_perspective>("shadow_perspective"_hs)
				.data<&ShadowComponentType::transforms>("transforms"_hs)
				//.data<&ShadowComponentType::shadow_map>("shadow_map"_hs)
				
				/*
				.ctor
				<
					decltype(ShadowComponentType::shadow_perspective),
					decltype(ShadowComponentType::transforms),
					decltype(ShadowComponentType::shadow_map)&&
				>()
				*/

				//.ctor<...>()
			;
		}
	}

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
	void reflect<LightProperties>()
	{
		engine_meta_type<LightProperties>()
			.data<&LightProperties::ambient>("ambient"_hs)
			.data<&LightProperties::diffuse>("diffuse"_hs)
			.data<&LightProperties::specular>("specular"_hs)
			
			//.data<nullptr, &LightProperties::max_brightness>("max_brightness"_hs)

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

			.ctor<decltype(DirectionalLightComponent::use_position)>()
		;
	}

	template <>
	void reflect<SpotLightComponent>()
	{
		engine_meta_type<SpotLightComponent>()
			.data<&SpotLightComponent::constant>("constant"_hs)
			.data<&SpotLightComponent::linear>("linear"_hs)
			.data<&SpotLightComponent::quadratic>("quadratic"_hs)
			.data<&SpotLightComponent::cutoff>("cutoff"_hs)
			.data<&SpotLightComponent::outer_cutoff>("outer_cutoff"_hs)

			.ctor
			<
				decltype(SpotLightComponent::constant),
				decltype(SpotLightComponent::linear),
				decltype(SpotLightComponent::quadratic),
				decltype(SpotLightComponent::cutoff),
				decltype(SpotLightComponent::outer_cutoff)
			>()
		;
	}

	template <>
	void reflect<PointLightComponent>()
	{
		engine_meta_type<PointLightComponent>()
			.data<&PointLightComponent::constant>("constant"_hs)
			.data<&PointLightComponent::linear>("linear"_hs)
			.data<&PointLightComponent::quadratic>("quadratic"_hs)

			.ctor
			<
				decltype(PointLightComponent::constant),
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
		engine_meta_type<LightComponent>()
			.base<LightProperties>()

			.data<&LightComponent::type>("type"_hs)
			.data<&LightComponent::set_casts_shadows, &LightComponent::get_casts_shadows>("casts_shadows"_hs)
			
			.ctor
			<
				decltype(LightProperties::ambient),
				decltype(LightProperties::diffuse),
				decltype(LightProperties::specular),
				
				decltype(LightComponent::type)
			>()

			.ctor
			<
				decltype(LightProperties::ambient),
				decltype(LightProperties::diffuse),
				decltype(LightProperties::specular),
				
				decltype(LightComponent::type),
				decltype(LightComponent::casts_shadows)
			>()
		;
	}

	template <>
	void reflect<DirectionalLightShadowComponent>()
	{
		world_impl::reflect_shadow_component<DirectionalLightShadowComponent>();
	}

	template <>
	void reflect<PointLightShadowComponent>()
	{
		world_impl::reflect_shadow_component<PointLightShadowComponent>();
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
		reflect<CameraProjection>();
		reflect<WorldProperties>();
		reflect<LightProperties>();

		// Components and behaviors:
		reflect<CameraComponent>();

		// NOTE: Handles several lighting-related types.
		reflect<LightComponent>();
		reflect<DirectionalLightComponent>();
		reflect<SpotLightComponent>();
		reflect<PointLightComponent>();

		reflect<DirectionalLightShadowComponent>();
		reflect<PointLightShadowComponent>();

		reflect_behaviors();

		// Systems:
		reflect<AnimationSystem>();
		reflect<PhysicsSystem>();
		reflect<MotionSystem>();

		// ...

		// Extensions:
		world_impl::reflect_extensions();
	}
}