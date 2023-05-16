#pragma once

#include <engine/world/types.hpp>
#include <engine/world/camera/components/camera_component.hpp>

#include <graphics/types.hpp>
#include <graphics/shadow_map.hpp>

namespace graphics
{
	class Context;
}

namespace engine
{
	template <typename TFormType, graphics::TextureType TextureRep, CameraComponent DefaultPerspective>
	struct ShadowComponentImpl
	{
		using TFormData = TFormType;
		using ShadowMap = graphics::ShadowMap;

		inline static constexpr auto default_shadow_perspective = DefaultPerspective;
		inline static constexpr auto map_type = TextureRep;

		inline ShadowComponentImpl
		(
			const std::shared_ptr<graphics::Context>& context,
			const TFormData& transforms,
			const math::vec2i& resolution,
			const CameraComponent& shadow_perspective=default_shadow_perspective
		) :
			transforms(transforms),
			shadow_map(context, resolution.x, resolution.y, map_type),
			shadow_perspective(shadow_perspective)
		{}

		inline ShadowComponentImpl
		(
			const TFormData& transforms,
			ShadowMap&& shadow_map,
			const CameraComponent& shadow_perspective=default_shadow_perspective
		) :
			transforms(transforms),
			shadow_map(std::move(shadow_map)),
			shadow_perspective(shadow_perspective)
		{}

		ShadowComponentImpl(ShadowComponentImpl&&) noexcept = default;

		ShadowComponentImpl& operator=(ShadowComponentImpl&&) noexcept = default;

		CameraComponent shadow_perspective;
		TFormData transforms;

		ShadowMap shadow_map;
	};
}