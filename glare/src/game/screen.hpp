#pragma once

#include <engine/types.hpp>

#include <graphics/types.hpp>
#include <graphics/gbuffer.hpp>

#include <tuple>
#include <cstdint>

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
	enum class GBufferDisplayMode : int // std::int32_t
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
			std::shared_ptr<graphics::Context> context;
			std::shared_ptr<graphics::Shader> display_shader;

			Size screen_size;
		public:
			GBufferDisplayMode display_mode = GBufferDisplayMode::Combined;

			// Deferred rendering screen buffer(s).
			graphics::GBuffer gbuffer;

			Screen(const std::shared_ptr<graphics::Context>& ctx, const Size& screen_size, const std::shared_ptr<graphics::Shader>& display_shader={});

			inline operator graphics::GBuffer& () { return gbuffer; }

			graphics::GBuffer& render(graphics::Shader& display_shader, const graphics::Viewport& viewport, graphics::GBuffer& gbuffer, GBufferDisplayMode display_mode);
			graphics::GBuffer& render(graphics::Shader& display_shader);
			graphics::GBuffer& render();

			std::tuple<Viewport, Size> update_viewport(const Window& window);
			std::tuple<Viewport, Size> update_viewport(const Window& window, engine::World& world, engine::Entity camera);

			void resize(Size size);
			void resize(int width, int height);

			Size get_size() const;

			void set_display_shader(const std::shared_ptr<graphics::Shader>& display_shader)
			{
				this->display_shader = display_shader;
			}

			const std::shared_ptr<graphics::Shader>& get_display_shader() const
			{
				return display_shader;
			}
	};
}