#pragma once

#include <graphics/math_types.hpp>
#include <graphics/texture_array.hpp>

#include <string_view>

namespace graphics
{
	class Context;
	class Shader;
}

namespace engine
{
	struct Config;

	struct WorldRenderState;
}

namespace game
{
	using RenderState = engine::WorldRenderState;

	// TODO: Determine if this class should be dissolved.
	class Effects
	{
		public:
			static std::string_view get_preprocessor();

			graphics::NamedTextureArrayRaw dynamic_texture_maps;
			graphics::ColorRGBA clear_color = { 0.0f, 0.0f, 0.0f, 1.0f };

			RenderState& bind(RenderState& render_state, const engine::Config& cfg);
	};
}