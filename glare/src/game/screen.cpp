#include "screen.hpp"

#include <app/window.hpp>

#include <engine/world/world.hpp>
#include <engine/world/camera.hpp>

#include <graphics/shader.hpp>

namespace game
{
	Screen::Screen(const ref<graphics::Context>& ctx, const Size& screen_size, const ref<graphics::Shader>& display_shader):
		context(ctx),
		gbuffer(ctx, screen_size),
		display_shader(display_shader)
	{
		if (!display_shader)
		{
			this->display_shader = memory::allocate<graphics::Shader>
			(
				ctx,

				"assets/shaders/display_texture.vert",
				"assets/shaders/display_texture.frag"
			);
		}
	}

	graphics::GBuffer& Screen::render(graphics::Shader& display_shader, const graphics::Viewport& viewport, graphics::GBuffer& gbuffer, GBufferDisplayMode display_mode)
	{
		if (display_mode == GBufferDisplayMode::None)
		{
			return gbuffer;
		}

		context->set_flags(graphics::ContextFlags::DepthTest, false);

		auto& shader = display_shader;

		context->clear_textures(false); // true

		context->use(shader, [&, this]()
		{
			auto get_texture = [&, this]() -> const graphics::Texture&
			{
				switch (display_mode)
				{
					case GBufferDisplayMode::Normal:
						return gbuffer.normal;
					case GBufferDisplayMode::AlbedoSpecular:
						return gbuffer.albedo_specular;

					case GBufferDisplayMode::Depth:
						if (gbuffer.depth)
						{
							return *gbuffer.depth;
						}

						break;

					/*
					case GBufferDisplayMode::ShadowMap:
					{
						auto light = world.get_by_name("shadow_test");

						if (light != engine::null)
						{
							auto& registry = world.get_registry();
							auto* shadows = registry.try_get<engine::DirectionLightShadows>(light);

							if (shadows)
							{
								const auto& depth = *shadows->shadow_map.get_depth_map();

								return depth;
							}
						}

						break;
					}
					*/

					/*
					case GBufferDisplayMode::RenderFlags:
						if (gbuffer.render_flags.has_value())
						{
							return *gbuffer.render_flags;
						}

						break;
					*/
				}

				if (gbuffer.position)
				{
					return *(gbuffer.position);
				}

				return gbuffer.albedo_specular;
			};

			auto fb_texture = context->use(get_texture());

			context->use(gbuffer.screen_quad, [&, this]()
			{
				context->draw();
			});
		});

		context->set_flags(graphics::ContextFlags::DepthTest, true);

		return gbuffer;
	}

	graphics::GBuffer& Screen::render(graphics::Shader& display_shader)
	{
		return render(display_shader, context->get_viewport(), gbuffer, display_mode);
	}

	graphics::GBuffer& Screen::render()
	{
		return render(*display_shader);
	}

	std::tuple<Screen::Viewport, Screen::Size> Screen::update_viewport(const Window& window)
	{
		Viewport viewport = {};

		int w_width, w_height;

		window.get_size(w_width, w_height);

		const auto old_viewport = context->get_viewport();

		viewport.set_size(static_cast<graphics::PointRect::Type>(w_width), static_cast<graphics::PointRect::Type>(w_height));

		if ((w_width != 0) && (w_height != 0))
		{
			if ((old_viewport.get_width() != w_width) || (old_viewport.get_height() != w_height))
			{
				context->set_viewport(viewport);
				
				//resize(w_width, w_height); // <-- Should already be triggered via window-event through `Game`.
			}
		}

		return { viewport, { w_width, w_height } };
	}

	std::tuple<Screen::Viewport, Screen::Size> Screen::update_viewport(const Window& window, engine::World& world, engine::Entity camera)
	{
		auto [viewport, w_size] = update_viewport(window);

		if ((std::get<0>(w_size) != 0) && (std::get<1>(w_size) != 0))
		{
			auto& camera_params = world.get_registry().get<engine::CameraParameters>(camera);

			camera_params.update_aspect_ratio(viewport.get_width(), viewport.get_height());
		}

		return { viewport, w_size };
	}
	
	void Screen::resize(Size size)
	{
		auto [width, height] = size;

		return resize(width, height);
	}

	void Screen::resize(int width, int height)
	{
		gbuffer.resize(width, height);
	}
}