#include <core.hpp>

#include <string>
#include <cmath>

#include <sdl2/SDL_video.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <math/math.hpp>

namespace unit_test
{
	class ShaderTest : public app::GraphicsApplication
	{
		public:
			template <typename T>
			using Var = graphics::ShaderVar<T>;

			using MeshType = graphics::Mesh<graphics::TextureVertex>;

			ref<graphics::Shader> test_shader;
			
			ref<graphics::Texture> texture1;
			ref<graphics::Texture> texture2;

			Var<int> texture1_sampler;
			Var<int> texture2_sampler;

			Var<math::mat4> projection;
			Var<math::mat4> view;
			Var<math::mat4> model;

			//graphics::ShaderVar<math::vec3> color;

			ref<MeshType> cube;

			ShaderTest(bool auto_execute=true)
			:
				GraphicsApplication("Shader Test", 1600, 900)
			{
				test_shader = memory::allocate<graphics::Shader>
				(
					graphics.context,
					
					"assets/unit_tests/shader_test/test.vs",
					"assets/unit_tests/shader_test/test.fs"
				);
				
				texture1 = memory::allocate<graphics::Texture>(graphics.context, "assets/unit_tests/shader_test/texture1.png");
				texture2 = memory::allocate<graphics::Texture>(graphics.context, "assets/unit_tests/shader_test/texture2.jpg");

				texture1_sampler = Var<int>("texture1", test_shader, 0);
				texture2_sampler = Var<int>("texture2", test_shader, 1);

				projection = Var<math::mat4>("projection", test_shader, {});
				view       = Var<math::mat4>("view",       test_shader, {});
				model      = Var<math::mat4>("model",      test_shader, {});

				//color = {"color", test_shader, {}};

				cube = memory::allocate<MeshType>
				(
					graphics.context,
					MeshType::Data
					{
						{
							{{{-0.5f, -0.5f, -0.5f}},  {0.0f, 0.0f}},
							{{{ 0.5f, -0.5f, -0.5f}},  {1.0f, 0.0f}},
							{{{ 0.5f,  0.5f, -0.5f}},  {1.0f, 1.0f}},
							{{{ 0.5f,  0.5f, -0.5f}},  {1.0f, 1.0f}},
							{{{-0.5f,  0.5f, -0.5f}},  {0.0f, 1.0f}},
							{{{-0.5f, -0.5f, -0.5f}},  {0.0f, 0.0f}},

							{{{-0.5f, -0.5f,  0.5f}},  {0.0f, 0.0f}},
							{{{ 0.5f, -0.5f,  0.5f}},  {1.0f, 0.0f}},
							{{{ 0.5f,  0.5f,  0.5f}},  {1.0f, 1.0f}},
							{{{ 0.5f,  0.5f,  0.5f}},  {1.0f, 1.0f}},
							{{{-0.5f,  0.5f,  0.5f}},  {0.0f, 1.0f}},
							{{{-0.5f, -0.5f,  0.5f}},  {0.0f, 0.0f}},

							{{{-0.5f,  0.5f,  0.5f}},  {1.0f, 0.0f}},
							{{{-0.5f,  0.5f, -0.5f}},  {1.0f, 1.0f}},
							{{{-0.5f, -0.5f, -0.5f}},  {0.0f, 1.0f}},
							{{{-0.5f, -0.5f, -0.5f}},  {0.0f, 1.0f}},
							{{{-0.5f, -0.5f,  0.5f}},  {0.0f, 0.0f}},
							{{{-0.5f,  0.5f,  0.5f}},  {1.0f, 0.0f}},

							{{{ 0.5f,  0.5f,  0.5f}},  {1.0f, 0.0f}},
							{{{ 0.5f,  0.5f, -0.5f}},  {1.0f, 1.0f}},
							{{{ 0.5f, -0.5f, -0.5f}},  {0.0f, 1.0f}},
							{{{ 0.5f, -0.5f, -0.5f}},  {0.0f, 1.0f}},
							{{{ 0.5f, -0.5f,  0.5f}},  {0.0f, 0.0f}},
							{{{ 0.5f,  0.5f,  0.5f}},  {1.0f, 0.0f}},

							{{{-0.5f, -0.5f, -0.5f}},  {0.0f, 1.0f}},
							{{{ 0.5f, -0.5f, -0.5f}},  {1.0f, 1.0f}},
							{{{ 0.5f, -0.5f,  0.5f}},  {1.0f, 0.0f}},
							{{{ 0.5f, -0.5f,  0.5f}},  {1.0f, 0.0f}},
							{{{-0.5f, -0.5f,  0.5f}},  {0.0f, 0.0f}},
							{{{-0.5f, -0.5f, -0.5f}},  {0.0f, 1.0f}},

							{{{-0.5f,  0.5f, -0.5f}},  {0.0f, 1.0f}},
							{{{ 0.5f,  0.5f, -0.5f}},  {1.0f, 1.0f}},
							{{{ 0.5f,  0.5f,  0.5f}},  {1.0f, 0.0f}},
							{{{ 0.5f,  0.5f,  0.5f}},  {1.0f, 0.0f}},
							{{{-0.5f,  0.5f,  0.5f}},  {0.0f, 0.0f}},
							{{{-0.5f,  0.5f, -0.5f}},  {0.0f, 1.0f}}
						},
						
						// Raw vertices; no index data.
						{}
					}
				);

				if (auto_execute)
				{
					execute();
				}
			}

			void update() override
			{
				
			}

			void render() override
			{
				auto& gfx = *graphics.canvas;
				auto& wnd = *window;

				gfx.clear(0, 0.5, 0, 1);

				graphics.context->use(*texture1, [this]()
				{
					graphics.context->use(*texture2, [this]()
					{
						graphics.context->use(*test_shader, [this]()
						{
							texture1_sampler.upload();
							texture2_sampler.upload();

							int width;
							int height;

							SDL_GetWindowSize(window->get_handle(), &width, &height);

							projection = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 100.0f);
							view = glm::translate(math::mat4(1.0f), math::vec3(0.0f, 0.0f, -3.0f));



							//color = { std::sin(milliseconds()), 0.5, 0.5 };
						});
					});
				});

				gfx.flip(wnd);
			}
	};
}