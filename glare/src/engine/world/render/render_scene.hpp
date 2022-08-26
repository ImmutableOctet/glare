#pragma once

namespace graphics
{
	class Canvas;
}

namespace engine
{
	class World;
	class ResourceManager;

	struct Resources;

	// Description of a target 'scene' and the
	// canvas/graphical-context we'll use to render it with.
	// See also: `RenderParameters`, which references this type and `WorldRenderer`.
	struct RenderScene
	{
		// A shared canvas/graphical-context for rendering.
		graphics::Canvas& canvas;

		// The world/scene containing the graphical entities we're targeting.
		engine::World& world; // const

		// Shared resources. (e.g. shaders, textures)
		Resources& resources; // const
	};
}