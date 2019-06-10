/*
	OpenGL graphics context implementation.
*/

#include <app/window.hpp>
#include <util/lib.hpp>

#include <graphics/context_state.hpp>
#include <graphics/context.hpp>

#include <graphics/vertex.hpp>
#include <graphics/canvas.hpp>
#include <graphics/shader.hpp>
#include <graphics/texture.hpp>
#include <graphics/pixelmap.hpp>

#include <graphics/native/opengl.hpp>

#include <sdl2/SDL_video.h>

namespace graphics
{
	// Most details of the active state is handled by OpenGL, but for
	// higher-level details, we keep track of state with this data-structure.
	class Driver
	{
		public:
			friend Context;

			using Handle = Context::Handle;

			static constexpr Handle NoHandle = Context::NoHandle;

			static constexpr int MAX_TEXTURES = 32; // GLenum
			static constexpr GLenum BASE_TEXTURE_INDEX = GL_TEXTURE0;
			static constexpr GLenum MAX_TEXTURE_INDEX = (BASE_TEXTURE_INDEX + MAX_TEXTURES); // GL_TEXTURE31;

			inline int max_textures() const { return MAX_TEXTURES; }
		protected:
			// SDL:
			SDL_GLContext sdl = nullptr;

			// State:
			GLenum texture_index = BASE_TEXTURE_INDEX;
		public:
			// Utility Code:
			static gl_uniform_location get_uniform(Shader& shader, raw_string name)
			{
				return glGetUniformLocation(shader.get_handle(), name);
			}

			// Returns the native OpenGL equivalent of 'VertexElementType'.
			static GLenum get_element_type(VertexElementType type)
			{
				switch (type)
				{
					case VertexElementType::Byte:    return GL_BYTE;
					case VertexElementType::UByte:   return GL_UNSIGNED_BYTE;
					case VertexElementType::Short:   return GL_SHORT;
					case VertexElementType::UShort:  return GL_UNSIGNED_SHORT;
					case VertexElementType::Int:     return GL_INT;
					case VertexElementType::UInt:    return GL_UNSIGNED_INT;
					case VertexElementType::Half:    return GL_HALF_FLOAT;
					case VertexElementType::Float:   return GL_FLOAT;
					case VertexElementType::Double:  return GL_DOUBLE;
				}

				return GL_NONE;
			}

			static Context::Handle compile_shader(const std::string& source, GLenum type) noexcept
			{
				const GLchar* source_raw = reinterpret_cast<const GLchar*>(source.c_str());

				// Allocate a native OpenGL shader source container.
				auto obj = glCreateShader(type);

				// Upload our shader source code.
				glShaderSource(obj, 1, &source_raw, nullptr); // source.length;

				// Compile the source code associated with our container.
				glCompileShader(obj);

				return obj;
			}
		protected:
			Driver(SDL_Window* window_handle)
				: sdl(SDL_GL_CreateContext(window_handle)) {}

			~Driver()
			{
				SDL_GL_DeleteContext(sdl);
			}
		public:
			// OpenGL API:
			static void bind_texture(GLenum texture_index, Handle texture)
			{
				// TODO: Add support for non-2D textures:
				glActiveTexture(texture_index);
				glBindTexture(GL_TEXTURE_2D, texture);
			}

			static void unbind_texture(GLenum texture_index)
			{
				bind_texture(texture_index, NoHandle);
			}

			GLenum push_texture(Handle texture)
			{
				if (texture_index >= MAX_TEXTURE_INDEX)
				{
					//reset_textures(false);

					return GL_NONE;
				}

				bind_texture(texture_index, texture);

				return ++texture_index;
			}

			// TODO: Observe performance difference between hard and soft unbinds of textures.
			GLenum pop_texture(bool hard_pop=true) // false
			{
				if (hard_pop)
				{
					unbind_texture(texture_index);
				}

				return --texture_index;
			}

			void reset_textures(bool hard_reset=false)
			{
				if (hard_reset)
				{
					for (GLenum i = BASE_TEXTURE_INDEX; i < MAX_TEXTURES; i++)
					{
						unbind_texture(texture_index);
					}
				}

				texture_index = BASE_TEXTURE_INDEX;
			}
	};

	static Driver* get_driver_raw(Context& ctx)
	{
		return reinterpret_cast<Driver*>(ctx.get_native());
	}

	static Driver& get_driver(Context& ctx)
	{
		return *get_driver_raw(ctx);
	}

	static Driver& get_driver(Context* ctx)
	{
		ASSERT(ctx);

		return get_driver(*ctx);
	}

	// Context API:
	Context::Context(app::Window& wnd, Backend gfx)
		: graphics_backend(gfx), state(memory::unique<Context::State>())
	{
		ASSERT(get_backend() == Backend::OpenGL);
		ASSERT(util::lib::establish_gl());

		native_context = new Driver(wnd.get_handle());

		glewInit();

		// TODO: Move this to a separate member-function:
		// Initial configuration:
		glEnable(GL_DEPTH_TEST);
	}

	Context::~Context()
	{
		//ASSERT(get_backend() == Backend::OpenGL);

		delete reinterpret_cast<Driver*>(native_context);
	}

	void Context::flip(app::Window& wnd)
	{
		SDL_GL_SwapWindow(wnd.get_handle());
	}

	void Context::clear(float red, float green, float blue, float alpha)
	{
		// TODO: Graphics Abstraction.
		glClearColor(red, green, blue, alpha);
		glClear(GL_COLOR_BUFFER_BIT);
	}

	void Context::clear_textures()
	{
		auto& driver = get_driver(this);

		driver.reset_textures(false); // true;
		state->clear_textures();
	}

	Shader& Context::bind(Shader& shader)
	{
		// Retrieve the currently bound shader.
		Shader& prev_shader = state->shader;

		if (prev_shader.get_handle() == shader.get_handle())
		{
			return shader;
		}

		// Bind the shader to the current graphical context.
		glUseProgram(shader.get_handle());

		// Assign the newly bound shader.
		state->shader = shader;

		// Return the previously bound shader.
		return prev_shader;
	}

	Texture& Context::bind(Texture& texture)
	{
		auto& driver = get_driver(this);
		auto& prev_texture = state->get_current_texture();

		if (prev_texture.get_handle() == texture.get_handle())
		{
			driver.pop_texture();

			return state->pop_texture();
		}

		state->push_texture(texture);
		driver.push_texture(texture.handle);
		
		return prev_texture;
	}

	// Mesh related:
	MeshComposition Context::generate_mesh(memory::memory_view vertices, std::size_t vertex_size, memory::array_view<VertexAttribute> attributes, memory::array_view<MeshIndex> indices) noexcept
	{
		ASSERT(vertices);
		ASSERT(vertex_size);
		ASSERT(attributes);

		GLMeshComposition mesh = {};

		const void* vertex_data_offset = 0;
		const bool indices_available = (indices);

		glGenVertexArrays(1, &mesh.VAO);

		// TODO: Handle shared VBOs for memory optimization:
		glGenBuffers(1, &mesh.VBO);

		if (indices_available)
		{
			glGenBuffers(1, &mesh.EBO);
		}

		// TODO: Potentially unify this implementation with standard bind operations.
		glBindVertexArray(mesh.VAO);


		// Vertex Data:
		glBindBuffer(GL_ARRAY_BUFFER, mesh.VBO);

		// TODO: Handle Static vs. Dynamic mesh data.
		glBufferData(GL_ARRAY_BUFFER, (vertices.size() * vertex_size), vertices.data(), GL_STATIC_DRAW);

		const auto* fields = attributes.data();

		// TODO: Look into unsigned sizes.
		for (std::size_t i = 0; i < (attributes.size()); i++) // static_cast<int>(...)
		{
			const auto& attribute = fields[i];

			const auto index = static_cast<GLuint>(i);
			const auto type = Driver::get_element_type(attribute.type);
			const auto element_count = static_cast<GLint>(attribute.num_elements);
			const auto stride = static_cast<GLsizei>(vertex_size);

			const void* offset = (reinterpret_cast<const std::int8_t*>(vertex_data_offset) + attribute.offset);

			ASSERT(type != GL_NONE);

			// TODO: Look into normalization flag for OpenGL driver.
			glVertexAttribPointer(index, element_count, type, GL_FALSE, stride, offset);
			glEnableVertexAttribArray(index);
		}

		// Index Data:
		if (indices_available)
		{
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.EBO);

			// TODO: Handle Static vs. Dynamic mesh data. (Less of a problem for indices)
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, (indices.size() * sizeof(MeshIndex)), indices.data(), GL_STATIC_DRAW);
		}

		return { mesh };
	}

	void Context::release_mesh(MeshComposition&& mesh)
	{
		auto& native_rep = mesh.gl;

		glDeleteVertexArrays(1, &native_rep.VAO);

		// Could be simplified to one GL call:
		glDeleteBuffers(1, &native_rep.EBO);
		glDeleteBuffers(1, &native_rep.VBO);
	}

	// Texture related:
	Context::Handle Context::generate_texture(const PixelMap& texture_data, TextureFlags flags)
	{
		ASSERT(texture_data);

		Handle texture = NoHandle;

		glGenTextures(1, &texture);

		// TODO: Add support for non-2D textures. (1D, 3D, etc)
		glBindTexture(GL_TEXTURE_2D, texture);

		// TODO: Add texture flag support:

		// Wrapping parameters:
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		// Filtering parameters:
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); // GL_NEAREST
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); // GL_NEAREST

		// TODO: Look into RGBA textures and greater-than 8-bit textures.
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texture_data.width(), texture_data.height(), 0, GL_RGB, GL_UNSIGNED_BYTE, texture_data.data());

		// TODO: Implement mip-mapping as a texture flag.
		glGenerateMipmap(GL_TEXTURE_2D);

		return texture;
	}

	void Context::release_texture(Handle&& handle)
	{
		glDeleteTextures(1, &handle);
	}

	// Shader related:
	Context::Handle Context::build_shader(const ShaderSource& source)
	{
		auto vertex   = Driver::compile_shader(source.vertex, GL_VERTEX_SHADER);
		auto fragment = Driver::compile_shader(source.fragment, GL_FRAGMENT_SHADER);

		// Allocate a shader-program object.
		auto program = glCreateProgram();

		// Attach our compiled shader objects.
		glAttachShader(program, vertex);
		glAttachShader(program, fragment);

		// Link together the program.
		glLinkProgram(program);

		glDeleteShader(vertex);
		glDeleteShader(fragment);

		return program;
	}

	void Context::release_shader(Context::Handle&& handle)
	{
		glDeleteProgram(handle);
	}

	bool Context::set_uniform(Shader& shader, raw_string name, int value)
	{
		glUniform1i(Driver::get_uniform(shader, name), value);

		return true;
	}

	bool Context::set_uniform(Shader& shader, raw_string name, bool value)
	{
		return set_uniform(shader, name, static_cast<int>(value));
	}

	bool Context::set_uniform(Shader& shader, raw_string name, float value)
	{
		glUniform1f(Driver::get_uniform(shader, name), value);

		return true;
	}
	
	bool Context::set_uniform(Shader& shader, raw_string name, const math::Vector2D& value)
	{
		glUniform2fv(Driver::get_uniform(shader, name), 1, &value[0]);

		return true;
	}

	bool Context::set_uniform(Shader& shader, raw_string name, const math::Vector3D& value)
	{
		glUniform3fv(Driver::get_uniform(shader, name), 1, &value[0]);

		return true;
	}

	bool Context::set_uniform(Shader& shader, raw_string name, const math::Vector4D& value)
	{
		glUniform4fv(Driver::get_uniform(shader, name), 1, &value[0]);

		return true;
	}

	bool Context::set_uniform(Shader& shader, raw_string name, const math::Matrix2x2& value)
	{
		glUniformMatrix2fv(Driver::get_uniform(shader, name), 1, GL_FALSE, &value[0][0]);

		return true;
	}

	bool Context::set_uniform(Shader& shader, raw_string name, const math::Matrix3x3& value)
	{
		glUniformMatrix3fv(Driver::get_uniform(shader, name), 1, GL_FALSE, &value[0][0]);

		return true;
	}

	bool Context::set_uniform(Shader& shader, raw_string name, const math::Matrix4x4& value)
	{
		glUniformMatrix4fv(Driver::get_uniform(shader, name), 1, GL_FALSE, &value[0][0]);

		return true;
	}
}