///*
#include "deferred_test.hpp"

#include <iostream>
#include <string>
#include <cmath>
#include <cstdlib>

#include <graphics/native/opengl.hpp>

#include <sdl2/SDL_video.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace glare::tests
{
	DeferredTest::DeferredTest(bool auto_execute)
		: GraphicsApplication("Deferred Shading", 1600, 900, app::WindowFlags::OpenGL)
	{
		using namespace graphics;

		static const std::string asset_folder = "assets/tests/deferred_test/";

		shaders.geometry_pass = Shader(graphics.context, (asset_folder + "g_buffer.vs"), (asset_folder + "g_buffer.fs"));
		shaders.lighting_pass = Shader(graphics.context, (asset_folder + "deferred_shading.vs"), (asset_folder + "deferred_shading.fs"));
		shaders.light_box     = Shader(graphics.context, (asset_folder + "deferred_light_box.vs"), (asset_folder + "deferred_light_box.fs"));

		graphics.context->use(shaders.lighting_pass, [&, this]()
		{
			// TODO: Refactor
			graphics.context->set_uniform(shaders.lighting_pass, "gPosition",   0);
			graphics.context->set_uniform(shaders.lighting_pass, "gNormal",     1);
			graphics.context->set_uniform(shaders.lighting_pass, "gAlbedoSpec", 2);
		});

		// Retrieve the default screen resolution:
		int screen_width, screen_height;

		window->get_size(screen_width, screen_height);

		// G Buffer:
		const auto gBuffer_flags = TextureFlags::None;

		gBuffer.framebuffer = FrameBuffer(graphics.context);

		gBuffer.position        = Texture(graphics.context, screen_width, screen_height, TextureFormat::RGB, ElementType::Half, gBuffer_flags);
		gBuffer.normal          = Texture(graphics.context, screen_width, screen_height, TextureFormat::RGB, ElementType::Half, gBuffer_flags);
		gBuffer.albedo_specular = Texture(graphics.context, screen_width, screen_height, TextureFormat::RGBA, ElementType::UByte, gBuffer_flags);

		gBuffer.screen_quad = Mesh::GenerateTexturedQuad(graphics.context);

		auto& fb = gBuffer.framebuffer;

		graphics.context->use(fb, [&, this]()
		{
			fb.attach(gBuffer.position);
			fb.attach(gBuffer.normal);
			fb.attach(gBuffer.albedo_specular);

			// TODO: Refactor*
			fb.attach(RenderBufferType::Depth, screen_width, screen_height);

			fb.link();
		});

		model = memory::allocate<Model>();
		
		*model = std::move(Model::Load(graphics.context, asset_folder + "nanosuit/nanosuit.obj"));

		make_lights();

		if (auto_execute)
		{
			execute();
		}
	}

	void DeferredTest::make_lights()
	{
		using namespace std;

		srand(13);

		for (unsigned int i = 0; i < NR_LIGHTS; i++)
		{
			// calculate slightly random offsets
			float xPos = (static_cast<float>(rand() % 100) / 100.0f) * 6.0f - 3.0f;
			float yPos = (static_cast<float>(rand() % 100) / 100.0f) * 6.0f - 4.0f;
			float zPos = (static_cast<float>(rand() % 100) / 100.0f) * 6.0f - 3.0f;

			lightPositions.push_back(glm::vec3(xPos, yPos, zPos));

			// also calculate random color
			float rColor = (static_cast<float>(rand() % 100) / 200.0f) + 0.5f; // between 0.5 and 1.0
			float gColor = (static_cast<float>(rand() % 100) / 200.0f) + 0.5f; // between 0.5 and 1.0
			float bColor = (static_cast<float>(rand() % 100) / 200.0f) + 0.5f; // between 0.5 and 1.0

			lightColors.push_back(glm::vec3(rColor, gColor, bColor));
		}
	}

	void DeferredTest::update()
	{

	}

	void DeferredTest::render()
	{
		auto& ctx = *graphics.context;
		auto& wnd = *window;

		int width, height;

		window->get_size(width, height);

		ctx.set_viewport(0, 0, width, height);

		// Geometry pass:
		ctx.use(gBuffer.framebuffer, [&, this]()
		{
			ctx.use(shaders.geometry_pass, [&, this]()
			{
				ctx.clear(0.0f, 0.0f, 0.0f, 1.0f, graphics::BufferType::Color | graphics::BufferType::Depth); // gfx

				uniforms.projection = glm::perspective(glm::radians(45.0f), window->horizontal_aspect_ratio(), 0.1f, 100.0f);
				uniforms.view = glm::translate(math::mat4(1.0f), math::vec3(0.0f, 0.0f, -4.0f));

				uniforms.model = glm::mat4(1.0f);
				uniforms.model = glm::translate(uniforms.model.get_value(), math::vec3(0.0, 0.0, -2));

				ctx.update(shaders.geometry_pass, uniforms.projection);
				ctx.update(shaders.geometry_pass, uniforms.view);
				ctx.update(shaders.geometry_pass, uniforms.model);

				auto meshes = model->get_meshes();

				for (auto& m : meshes)
				{
					auto& mesh = *m.first;

					ctx.use(mesh, [&, this]()
					{
						ctx.draw();
					});
				}
			});
		});

		ctx.clear(0.0f, 0.0f, 0.0f, 1.0f, graphics::BufferType::Color | graphics::BufferType::Depth);

		// Lighting pass:
		ctx.use(shaders.lighting_pass, [&, this]()
		{
			auto gPosition = ctx.use(gBuffer.position);
			auto gNormal   = ctx.use(gBuffer.normal);
			auto gAlbedoSpec = ctx.use(gBuffer.albedo_specular);

			// send light relevant uniforms
			for (unsigned int i = 0; i < lightPositions.size(); i++)
			{
				ctx.set_uniform(shaders.lighting_pass, ("lights[" + std::to_string(i) + "].Position").c_str(), lightPositions[i]);
				ctx.set_uniform(shaders.lighting_pass, ("lights[" + std::to_string(i) + "].Color").c_str(), lightColors[i]);

				// update attenuation parameters and calculate radius
				const float constant = 1.0; // note that we don't send this to the shader, we assume it is always 1.0 (in our case)
				const float linear = 0.7;
				const float quadratic = 1.8;

				ctx.set_uniform(shaders.lighting_pass, ("lights[" + std::to_string(i) + "].Linear").c_str(), linear);
				ctx.set_uniform(shaders.lighting_pass, ("lights[" + std::to_string(i) + "].Quadratic").c_str(), quadratic);
				
				// then calculate radius of light volume/sphere
				const float maxBrightness = std::fmaxf(std::fmaxf(lightColors[i].r, lightColors[i].g), lightColors[i].b);
				float radius = (-linear + std::sqrt(linear * linear - 4 * quadratic * (constant - (256.0f / 5.0f) * maxBrightness))) / (2.0f * quadratic);

				ctx.set_uniform(shaders.lighting_pass, ("lights[" + std::to_string(i) + "].Radius").c_str(), radius);
			}

			//shaderLightingPass.setVec3("viewPos", camera.Position);

			ctx.use(gBuffer.screen_quad, [&, this]()
			{
				ctx.draw();
			});
		});

		ctx.flip(wnd);
	}
}
//*/