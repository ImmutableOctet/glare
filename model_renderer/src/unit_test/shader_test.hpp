#include <core.hpp>

#include <string>
#include <cmath>

#include <glm/vec3.hpp>

namespace unit_test
{
	class ShaderTest : public app::GraphicsApplication
	{
		public:
			using MeshType = graphics::Mesh<graphics::TextureVertex>;

			ref<graphics::Shader> test_shader;
			graphics::ShaderVar<glm::vec3> color;

			ref<MeshType> cube;

			ShaderTest(bool auto_execute=true)
			:
				GraphicsApplication("Shader Test", 1600, 900),
				
				test_shader
				(
					memory::allocate<graphics::Shader>
					(
						graphics.context,
						"assets/unit_tests/shader_test/test.vs",
						"assets/unit_tests/shader_test/test.fs"
					)
				),

				color("color", test_shader, {})
			{


				cube = memory::allocate<MeshType>
				(
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

				graphics.context->use(*test_shader, [this]()
				{
					color = { std::sin(milliseconds()), 0.5, 0.5 };
				});

				gfx.flip(wnd);
			}
	};
}