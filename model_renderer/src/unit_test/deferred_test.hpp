#pragma once

#include <core.hpp>
#include <math/math.hpp>

#include <glm/vec3.hpp>

namespace unit_test
{
	class DeferredTest : public app::GraphicsApplication
	{
		public:
			struct
			{
				graphics::Shader geometry_pass;
				graphics::Shader lighting_pass;
				graphics::Shader light_box;
			} shaders;

			struct
			{
				graphics::FrameBuffer framebuffer;

				graphics::Texture position;
				graphics::Texture normal;
				graphics::Texture albedo_specular;

				graphics::Mesh screen_quad;
			} gBuffer;

			struct
			{
				Uniform<math::mat4> projection = {"projection"};
				Uniform<math::mat4> view = {"view"};
				Uniform<math::mat4> model = {"model"};
			} uniforms;

			ref<graphics::Model> model;

			std::vector<glm::vec3> lightPositions;
			std::vector<glm::vec3> lightColors;

			const unsigned int NR_LIGHTS = 32;

			void make_lights();

			DeferredTest(bool auto_execute=true);

			void update() override;
			void render() override;
	};
}