#pragma once

#include <tuple>

#include <types.hpp>
#include <engine/types.hpp>
#include <graphics/types.hpp>

#include <graphics/gbuffer.hpp>

namespace app
{
	class Window;
}

namespace graphics
{
	class Context;
}

namespace engine
{
	class World;
}

namespace game
{
	// Used for debugging purposes.
	enum class GBufferDisplayMode : int
	{
		Combined,
		None=Combined,

		Position,
		Normal,
		//ShadowMap,
		AlbedoSpecular,
		Depth,
		//RenderFlags,

		// Number of display modes.
		Modes,
	};

	class Screen
	{
		public:
			using Window = app::Window;
			using Size = std::tuple<int, int>; // math::vec2i;
			using Viewport = graphics::Viewport;

		protected:
			ref<graphics::Context> context;
			ref<graphics::Shader> display_shader;

		public:
			GBufferDisplayMode display_mode = GBufferDisplayMode::Combined;

			// Deferred rendering screen buffer(s).
			graphics::GBuffer gbuffer;

			Screen(const ref<graphics::Context>& ctx, const Size& screen_size, const ref<graphics::Shader>& display_shader={});

			inline operator graphics::GBuffer& () { return gbuffer; }

			graphics::GBuffer& render(graphics::Shader& display_shader, const graphics::Viewport& viewport, graphics::GBuffer& gbuffer, GBufferDisplayMode display_mode);
			graphics::GBuffer& render(graphics::Shader& display_shader);
			graphics::GBuffer& render();

			std::tuple<Viewport, Size> update_viewport(const Window& window);
			std::tuple<Viewport, Size> update_viewport(const Window& window, engine::World& world, engine::Entity camera);

			void resize(Size size);
			void resize(int width, int height);

			void set_display_shader(const ref<graphics::Shader>& display_shader)
			{
				this->display_shader = display_shader;
			}

			const ref<graphics::Shader>& get_display_shader() const
			{
				return display_shader;
			}
	};
}