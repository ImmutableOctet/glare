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

			static constexpr gl_uniform_location InvalidUniform = -1;

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

			// Returns the native OpenGL equivalent of 'ElementType'.
			static GLenum get_element_type(ElementType type)
			{
				switch (type)
				{
					case ElementType::Byte:    return GL_BYTE;
					case ElementType::UByte:   return GL_UNSIGNED_BYTE;
					case ElementType::Short:   return GL_SHORT;
					case ElementType::UShort:  return GL_UNSIGNED_SHORT;
					case ElementType::Int:     return GL_INT;
					case ElementType::UInt:    return GL_UNSIGNED_INT;
					case ElementType::Half:    return GL_HALF_FLOAT;
					case ElementType::Float:   return GL_FLOAT;
					case ElementType::Double:  return GL_DOUBLE;
				}

				return GL_NONE;
			}

			// TODO: Look into 10-bit color options, etc:
			static GLenum get_texture_format(const PixelMap& texture_data)
			{
				// Check which color channels are enabled.
				switch (texture_data.channels())
				{
					case 1:
						return GL_RED;
					case 2:
						return GL_RG;
					case 3:
						return GL_RGB;
					case 4:
						return GL_RGBA;
				}

				return GL_NONE;
			}

			static GLenum get_primitive(Primitive primitive)
			{
				switch (primitive)
				{
					case Primitive::Point:           return GL_POINTS;
					case Primitive::Line:            return GL_LINE;
					case Primitive::LineLoop:        return GL_LINE_LOOP;
					case Primitive::LineStrip:       return GL_LINE_STRIP;
					case Primitive::Triangle:        return GL_TRIANGLES;
					case Primitive::TriangleStrip:   return GL_TRIANGLE_STRIP;
					case Primitive::TriangleFan:     return GL_TRIANGLE_FAN;
					case Primitive::Quad:            return GL_QUADS;
					case Primitive::QuadStrip:       return GL_QUAD_STRIP;
					case Primitive::Polygon:         return GL_POLYGON;
				}

				return GL_NONE;
			}

			static Context::Handle compile_shader(const std::string& source, GLenum type) noexcept
			{
				const GLchar* source_raw = reinterpret_cast<const GLchar*>(source.c_str());

				// Allocate a native OpenGL shader source container.
				auto obj = glCreateShader(type);

				if (obj == 0) // NULL
				{
					return {};
				}

				// Upload our shader source code.
				glShaderSource(obj, 1, &source_raw, nullptr); // source.length;

				// Compile the source code associated with our container.
				glCompileShader(obj);

				GLint success;

				glGetShaderiv(obj, GL_COMPILE_STATUS, &success); //ASSERT(success);

				if (!success)
				{
					glDeleteShader(obj);

					return {};
				}

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

	void Context::configure_3D()
	{
		glEnable(GL_DEPTH_TEST);
	}

	void Context::flip(app::Window& wnd)
	{
		SDL_GL_SwapWindow(wnd.get_handle());
	}

	void Context::clear(float red, float green, float blue, float alpha, BufferType buffer_type)
	{
		// TODO: Look into clear color state. (Likely to be added to a 'Canvas' type)
		glClearColor(red, green, blue, alpha);

		GLbitfield buffer_flags = 0; // {};

		if ((buffer_type & BufferType::Color)) { buffer_flags |= GL_COLOR_BUFFER_BIT; }
		if ((buffer_type & BufferType::Depth)) { buffer_flags |= GL_DEPTH_BUFFER_BIT; }

		if (buffer_flags == 0)
		{
			buffer_flags = GL_COLOR_BUFFER_BIT;
		}

		glClear(buffer_flags);
	}

	void Context::draw()
	{
		const Mesh& mesh = state->mesh;

		const auto primitive_type = Driver::get_primitive(mesh.get_primitive());
		const auto offset = static_cast<GLint>(mesh.offset());
		const auto count = static_cast<GLsizei>(mesh.size());

		glDrawArrays(primitive_type, offset, count);
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

	Mesh& Context::bind(Mesh& mesh)
	{
		// Retrieve the currently bound mesh.
		Mesh& prev_mesh = state->mesh;

		const auto& native_rep = mesh.get_composition().gl;

		glBindVertexArray(native_rep.VAO);

		// Assign the newly bound mesh.
		state->mesh = mesh;

		// Return the previously bound mesh.
		return prev_mesh;
	}

	// Mesh related:
	MeshComposition Context::generate_mesh(memory::memory_view vertices, std::size_t vertex_size, memory::array_view<VertexAttribute> attributes, memory::array_view<MeshIndex> indices) noexcept
	{
		ASSERT(vertices);
		ASSERT(vertex_size);
		ASSERT(attributes);

		GLComposition mesh = {};

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
		const auto& native_rep = mesh.gl;

		// TODO: Make sure this is valid behavior when 'native_rep' contains default constructed values.
		glDeleteVertexArrays(1, &native_rep.VAO);

		// Could be simplified to one GL call:
		glDeleteBuffers(1, &native_rep.EBO);
		glDeleteBuffers(1, &native_rep.VBO);
	}

	// Texture related:
	Context::Handle Context::generate_texture(const PixelMap& texture_data, ElementType channel_type, TextureFlags flags)
	{
		ASSERT(texture_data);

		// At this time, no more than four color channels are supported.
		ASSERT(texture_data.channels() <= 4);

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

		// TODO: Review behavior for Depth and Stencil buffers.
		bool is_depth_map = ((flags & TextureFlags::DepthMap));

		// For now, Depth maps are not supported.
		ASSERT(!is_depth_map);

		// TODO: Greater-than 8-bit textures.
		const auto texture_format = Driver::get_texture_format(texture_data);
		const auto element_type = Driver::get_element_type(channel_type);

		glTexImage2D(GL_TEXTURE_2D, 0, texture_format, texture_data.width(), texture_data.height(), 0, texture_format, element_type, texture_data.data());

		// TODO: Implement mip-mapping as a texture flag.
		glGenerateMipmap(GL_TEXTURE_2D);

		return texture;
	}

	void Context::release_texture(Handle&& handle)
	{
		glDeleteTextures(1, &handle);
	}

	// Shader related:
	Context::Handle Context::build_shader(const ShaderSource& source) noexcept
	{
		auto vertex   = Driver::compile_shader(source.vertex, GL_VERTEX_SHADER);
		auto fragment = Driver::compile_shader(source.fragment, GL_FRAGMENT_SHADER);

		// TODO: Look into proper error handling if possible.
		ASSERT(vertex);
		ASSERT(fragment);

		// Allocate a shader-program object.
		auto program = glCreateProgram();

		// Attach our compiled shader objects.
		glAttachShader(program, vertex);
		glAttachShader(program, fragment);

		// Link together the program.
		glLinkProgram(program);
		
		GLint success;
		glGetProgramiv(program, GL_LINK_STATUS, &success);

		// TODO: Look into proper error handling for link-failure if possible.
		ASSERT(success);

		glDeleteShader(vertex);
		glDeleteShader(fragment);

		return program;
	}

	void Context::release_shader(Context::Handle&& handle)
	{
		glDeleteProgram(handle);
	}

	bool Context::set_uniform(Shader& shader, raw_string name, bool value)
	{
		return set_uniform(shader, name, static_cast<int>(value));
	}

	bool Context::set_uniform(Shader& shader, raw_string name, int value)
	{
		auto uniform = Driver::get_uniform(shader, name);

		if (uniform == Driver::InvalidUniform)
		{
			return false;
		}

		glUniform1i(uniform, value);

		return true;
	}

	bool Context::set_uniform(Shader& shader, raw_string name, float value)
	{
		auto uniform = Driver::get_uniform(shader, name);

		if (uniform == Driver::InvalidUniform)
		{
			return false;
		}

		glUniform1f(uniform, value);

		return true;
	}
	
	bool Context::set_uniform(Shader& shader, raw_string name, const math::Vector2D& value)
	{
		auto uniform = Driver::get_uniform(shader, name);

		if (uniform == Driver::InvalidUniform)
		{
			return false;
		}

		glUniform2fv(uniform, 1, &value[0]);

		return true;
	}

	bool Context::set_uniform(Shader& shader, raw_string name, const math::Vector3D& value)
	{
		auto uniform = Driver::get_uniform(shader, name);

		if (uniform == Driver::InvalidUniform)
		{
			return false;
		}

		glUniform3fv(uniform, 1, &value[0]);

		return true;
	}

	bool Context::set_uniform(Shader& shader, raw_string name, const math::Vector4D& value)
	{
		auto uniform = Driver::get_uniform(shader, name);

		if (uniform == Driver::InvalidUniform)
		{
			return false;
		}

		glUniform4fv(uniform, 1, &value[0]);

		return true;
	}

	bool Context::set_uniform(Shader& shader, raw_string name, const math::Matrix2x2& value)
	{
		auto uniform = Driver::get_uniform(shader, name);

		if (uniform == Driver::InvalidUniform)
		{
			return false;
		}

		glUniformMatrix2fv(uniform, 1, GL_FALSE, &value[0][0]);

		return true;
	}

	bool Context::set_uniform(Shader& shader, raw_string name, const math::Matrix3x3& value)
	{
		auto uniform = Driver::get_uniform(shader, name);

		if (uniform == Driver::InvalidUniform)
		{
			return false;
		}

		glUniformMatrix3fv(uniform, 1, GL_FALSE, &value[0][0]);

		return true;
	}

	bool Context::set_uniform(Shader& shader, raw_string name, const math::Matrix4x4& value)
	{
		auto uniform = Driver::get_uniform(shader, name);

		if (uniform == Driver::InvalidUniform)
		{
			return false;
		}

		glUniformMatrix4fv(uniform, 1, GL_FALSE, &value[0][0]);

		return true;
	}
}