#pragma once

#include "components/light_component.hpp"

#include <engine/world/entity.hpp>
#include <engine/world/camera.hpp>
//#include <graphics/types.hpp>
#include <graphics/cubemap_transform.hpp>

#include <graphics/types.hpp>

#include <optional>

namespace graphics
{
	class Context;
	class Shader;
}

namespace engine
{
	Entity create_directional_light(World& world, const math::Vector& position, const LightProperties& properties={}, const LightProperties::Directional& advanced_properties={}, Entity parent=null, bool debug_mode=false, pass_ref<graphics::Shader> debug_shader={});
	Entity create_spot_light(World& world, const math::Vector& position, const LightProperties& properties={}, const LightProperties::Spot& advanced_properties={}, Entity parent=null, bool debug_mode=false, pass_ref<graphics::Shader> debug_shader={});
	Entity create_point_light(World& world, const math::Vector& position, const LightProperties& properties={}, const LightProperties::Point& advanced_properties={}, Entity parent=null, bool debug_mode=false, pass_ref<graphics::Shader> debug_shader={});

	// Attaches point-light shadows to the 'light' entity specified.
	bool attach_shadows(World& world, Entity light, const math::vec2i& resolution, float shadow_range=1000.0f, std::optional<LightType> shadow_light_type=std::nullopt);
	bool attach_shadows(World& world, Entity light, LightType light_type, std::optional<math::vec2i> resolution=std::nullopt, std::optional<CameraParameters> perspective_cfg=std::nullopt, bool update_aspect_ratio=true, bool conform_to_light_type=false);

	// Update shadow-map state (LightShadows, etc.) based on the type of light specified.
	void update_shadows(World& world, Entity light);
}