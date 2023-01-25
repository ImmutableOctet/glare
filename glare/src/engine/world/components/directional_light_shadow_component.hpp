#pragma once

#include "shadow_component_impl.hpp"

#include <math/types.hpp>
#include <graphics/types.hpp>

namespace engine
{
	struct DirectionalLightShadowComponent
		: ShadowComponentImpl
		<
			math::Matrix4x4,
			graphics::TextureType::Texture2D,
			
			CameraComponent
			{
				90.0f, // 45.0f
				0.0f, 1000.0f, // 1.0f, 1000.0f,
				1.0f,
				CameraProjection::Orthographic // CameraProjection::Perspective
			}
		>
	{
		using ShadowComponentImpl::ShadowComponentImpl;
	};
}