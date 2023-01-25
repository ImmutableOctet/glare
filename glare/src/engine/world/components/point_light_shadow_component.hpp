#pragma once

#include "shadow_component_impl.hpp"

#include <graphics/types.hpp>
#include <graphics/cubemap_transform.hpp>

namespace engine
{
	struct PointLightShadowComponent
		: ShadowComponentImpl
		<
			graphics::CubeMapTransforms,
			graphics::TextureType::CubeMap,
			
			CameraComponent
			{
				90.0f,
				1.0f, 1000.0f,
				1.0,
				CameraProjection::Perspective
			}
		>
	{
		using ShadowComponentImpl::ShadowComponentImpl;
	};
}