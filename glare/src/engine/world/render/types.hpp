#pragma once

namespace graphics
{
	class Context;
	class Shader;
	class Canvas;
	class GBuffer;

	struct PointRect;

	using Viewport = PointRect;
}

namespace engine
{
	struct WorldRenderState;

	using RenderBuffer   = graphics::GBuffer;
	using RenderState    = WorldRenderState;
	using RenderViewport = graphics::Viewport;
}