#pragma once

#include "types.hpp"

#include <engine/types.hpp>

namespace engine
{
	struct RenderScene;

	// Scene/graphical-context descriptor + additional
	// context fleshing out a rendering phase's base input.
	struct RenderParameters
	{
		// The target scene we are rendering.
		const RenderScene& scene;

		// The camera we're rendering with.
		Entity camera;

		// The screen/buffer we're rendering to.
		RenderBuffer& gbuffer;

		// The viewport we're populating within the screen/gbuffer.
		const RenderViewport& viewport; // RenderViewport viewport;

		// Shared state between rendering phases.
		RenderState& render_state;
	};
}