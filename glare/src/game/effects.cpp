#include "effects.hpp"

#include <engine/world/render/world_render_state.hpp>
#include <engine/config.hpp>

namespace game
{
	std::string_view Effects::get_preprocessor()
	{
		return
		"\n#define LAYER_DEPTH_ENABLED 1\n"
		//"\n#define LAYER_POSITION_ENABLED 1\n"
		;
	}

	RenderState& Effects::bind(RenderState& render_state, const engine::Config& cfg)
	{
		render_state.dynamic_textures = { &dynamic_texture_maps };

		return render_state;
	}
}