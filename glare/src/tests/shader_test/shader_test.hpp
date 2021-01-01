#pragma once

#include <core.hpp>
#include <math/math.hpp>
#include <glm/fwd.hpp>

//#include <sdl2/SDL_stdinc.h>

namespace glare::tests
{
	class ShaderTest : public app::GraphicsApplication
	{
		public:
			ref<graphics::Shader> test_shader;
			
			graphics::Texture texture1;
			graphics::Texture texture2;

			struct
			{
				Uniform<math::mat4> projection = {"projection"};
				Uniform<math::mat4> view = {"view"};
				Uniform<math::mat4> model = {"model"};
			} uniforms;

			//graphics::ShaderVar<math::vec3> color;

			graphics::Mesh cube;
			graphics::Mesh quad;

			const std::uint8_t* key_state = nullptr;

			//glm::quat rotation;

			ShaderTest(bool auto_execute=true);

			void update() override;
			void render() override;

			virtual void on_keydown(const keyboard_event_t& event) override;
			virtual void on_keyup(const keyboard_event_t& event) override;
	};
}