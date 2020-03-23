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
#include <graphics/framebuffer.hpp>
#include <graphics/renderbuffer.hpp>
#include <graphics/texture.hpp>
#include <graphics/pixelmap.hpp>

#include <graphics/native/opengl.hpp>

#include <sdl2/SDL_video.h>

#include <utility>
#include <variant>
#include <vector>
#include <unordered_map>
//#include <stack>
#include <type_traits>

namespace graphics
{
	using Driver = Context::Driver;

	// Most details of the active state is handled by OpenGL, but for
	// higher-level details, we keep track of state with this data-structure.
	class Context::Driver
	{
		public:
			friend Context;

			using Handle = Context::Handle;
			using Flags = Context::Flags;

			static constexpr Handle NoHandle = Context::NoHandle;

			static constexpr int MAX_TEXTURES = 32; // GLenum // std::size_t
			static constexpr GLenum BASE_TEXTURE_INDEX = GL_TEXTURE0;
			static constexpr GLenum MAX_TEXTURE_INDEX = (BASE_TEXTURE_INDEX + MAX_TEXTURES); // GL_TEXTURE31;

			static constexpr gl_uniform_location InvalidUniform = -1;

			inline int max_textures() const { return MAX_TEXTURES; }
		protected:
			// SDL:
			SDL_GLContext SDL = nullptr;

			// State:
			GLenum gl_texture_id = BASE_TEXTURE_INDEX; // 0;

			std::vector<Handle> texture_stack;

			// TODO: Implement wrapping behavior for bound textures.
			//bool wrap_texture_indices = true;
		private:
			// Used internally to mitigate constant allocation of buffers for texture-array description.
			std::vector<int> _texture_assignment_buffer;

			using UniformLocationMap       = std::unordered_map<std::string_view, gl_uniform_location>;
			using UniformLocationContainer = std::unordered_map<Handle, UniformLocationMap>;

			UniformLocationContainer _uniform_cache;
		protected:
			static int gl_texture_id_to_index(GLenum texture_id)
			{
				return static_cast<int>((texture_id - BASE_TEXTURE_INDEX));
			}

			static GLenum gl_index_to_texture_id(int texture_index)
			{
				return static_cast<GLenum>((texture_index + BASE_TEXTURE_INDEX));
			}
		public:
			/*
			struct texture_format_properties
			{
				GLenum element_type;
				GLenum format;
				GLenum layout;

				int channels; // std::uint8_t
			};
			*/

			// Utility Code:
			gl_uniform_location get_uniform(Shader& shader, raw_string name)
			{
				return glGetUniformLocation(shader.get_handle(), name);
			}

			// Applies texture flags to the currently bound texture.
			static void apply_texture_flags(TextureFlags flags)
			{
				// Wrapping parameters:
				if ((flags & TextureFlags::WrapS))
				{
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
				}

				if ((flags & TextureFlags::WrapT))
				{
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
				}

				// TODO: Review filtering behavior.

				// Filtering parameters:
				if ((flags & TextureFlags::LinearFiltering))
				{
					if ((flags & TextureFlags::MipMap))
					{
						glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
						glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
					}
					else
					{
						glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
						glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
					}
				}
				else
				{
					if ((flags & TextureFlags::MipMap))
					{
						glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
						glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST_MIPMAP_NEAREST);
					}
					else
					{
						glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
						glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
					}
				}
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

			static GLenum get_format_from_channels(int channels)
			{
				switch (channels)
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

			static int get_format_channels(TextureFormat format)
			{
				return PixelMap::get_format_channels(format);
			}

			// TODO: Review other potential formats if needed. (May already work as intended)
			static GLenum get_texture_format(TextureFormat format, ElementType element_type, bool exact_format)
			{
				//auto native_format_raw = get_format_from_channels(get_format_channels(format));
				auto native_format_raw = get_texture_layout(format);

				if (exact_format)
				{
					switch (native_format_raw)
					{
						case GL_RGB:
							switch (element_type)
							{
								case ElementType::Half:
									return GL_RGB16F;
								case ElementType::Float:
									return GL_RGB32F;
							}

							break;
						case GL_DEPTH_COMPONENT:
							switch (element_type)
							{
								case ElementType::Half:
								case ElementType::Short:
								case ElementType::UShort:
									return GL_DEPTH_COMPONENT16;
								
								// Special-case usage of 'UInt24':
								case ElementType::UInt24:
									return GL_DEPTH_COMPONENT24;

								// NOTE: 32-bit depth-buffer support is driver-dependent.
								case ElementType::Float:
								case ElementType::Int:
								case ElementType::UInt:
									return GL_DEPTH_COMPONENT32;
							}

							break;
						case GL_STENCIL_INDEX:
							switch (element_type)
							{
								case ElementType::Bit:
									return GL_STENCIL_INDEX1;
								case ElementType::Nibble:
									return GL_STENCIL_INDEX4;
								case ElementType::Short:
								case ElementType::UShort:
									return GL_STENCIL_INDEX16;
							}

							break;
						case GL_DEPTH_STENCIL:
							switch (element_type)
							{
								case ElementType::Int:
								case ElementType::UInt:
									return GL_DEPTH24_STENCIL8;
								case ElementType::Int32_8:
									return GL_DEPTH32F_STENCIL8;
							}

							break;
					}
				}

				return native_format_raw;
			}

			static GLenum get_texture_format(const PixelMap& texture_data, ElementType element_type, bool exact_format)
			{
				return get_texture_format(texture_data.format(), element_type, exact_format);
			}

			static GLenum get_texture_layout(TextureFormat format)
			{
				switch (format)
				{
					case TextureFormat::R:
						return get_format_from_channels(1);
					case TextureFormat::RG:
						return get_format_from_channels(2);
					case TextureFormat::RGB:
						return get_format_from_channels(3);
					case TextureFormat::RGBA:
						return get_format_from_channels(4);
					case TextureFormat::Depth:
						return GL_DEPTH_COMPONENT;
					case TextureFormat::Stencil:
						return GL_STENCIL_INDEX;
					case TextureFormat::DepthStencil:
						return GL_DEPTH_STENCIL;
				}

				return GL_NONE;
			}

			static GLenum get_texture_layout(const PixelMap& texture_data)
			{
				return get_texture_layout(texture_data.format());
			}

			static GLenum get_renderbuffer_attachment(RenderBufferType type)
			{
				switch (type)
				{
					case RenderBufferType::Color:
						return GL_COLOR_ATTACHMENT0;
					case RenderBufferType::Depth:
						return GL_DEPTH_ATTACHMENT;
					case RenderBufferType::Stencil:
						return GL_STENCIL_ATTACHMENT;
					case RenderBufferType::DepthStencil:
						return GL_DEPTH_STENCIL_ATTACHMENT;
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
			
			static Flags handle_flag(ContextState& state, Flags flags, Flags target_flag, GLenum gl_target, bool value)
			{
				if ((flags & target_flag))
				{
					if (value)
					{
						glEnable(gl_target);

						state.flags |= target_flag;
					}
					else
					{
						state.flags &= target_flag;

						glDisable(gl_target);
					}
				}

				return state.flags;
			}

			static Flags set_flags(ContextState& state, Flags flags, bool value)
			{
				// TODO: Look into compile-time maps.
				// (Data driven solution vs. code driven solution)
				handle_flag(state, flags, Flags::DepthTest, GL_DEPTH_TEST, value);

				// Depth-testing:
				if ((flags & Flags::DepthTest))
				{
					glDepthFunc(GL_LESS);
				}

				// Face culling:
				if ((flags & Flags::FaceCulling))
				{
					// Discard back-faces, keep front-faces.
					glCullFace(GL_FRONT_AND_BACK); // GL_FRONT // GL_BACK
				}

				handle_flag(state, flags, Flags::FaceCulling, GL_CULL_FACE, value);

				return state.get_flags();
			}
		protected:
			Driver(SDL_Window* window_handle)
				: SDL(SDL_GL_CreateContext(window_handle)),
					texture_stack(MAX_TEXTURES, NoHandle)
			{
				_texture_assignment_buffer.reserve(MAX_TEXTURES);
			}

			~Driver()
			{
				SDL_GL_DeleteContext(SDL);
			}
		public:
			// OpenGL API:
			static void bind_texture(GLenum gl_texture_id, Handle texture)
			{
				// TODO: Add support for non-2D textures:
				glActiveTexture(gl_texture_id);
				glBindTexture(GL_TEXTURE_2D, texture);
			}

			static void unbind_texture(GLenum gl_texture_id)
			{
				bind_texture(gl_texture_id, NoHandle);
			}

			GLenum push_texture(Handle texture)
			{
				///ASSERT(gl_texture_id < MAX_TEXTURE_INDEX);

				if (gl_texture_id >= MAX_TEXTURE_INDEX)
				{
					//reset_textures(false);

					return GL_NONE;
				}

				bind_texture(gl_texture_id, texture);

				texture_stack[gl_texture_id_to_index(gl_texture_id)] = texture;

				return ++gl_texture_id;
			}

			// TODO: Observe performance difference between hard and soft unbinds of textures.
			GLenum pop_texture(bool hard_pop=true) // false
			{
				if (hard_pop)
				{
					unbind_texture(gl_texture_id);
				}

				texture_stack[gl_texture_id_to_index(gl_texture_id)] = NoHandle;

				return --gl_texture_id;
			}

			// If the specified texture-handle could not be found, '-1' is returned.
			int get_texture_index(Handle texture)
			{
				for (auto i = 0; i < texture_stack.size(); i++)
				{
					if (texture_stack[i] == texture)
					{
						return i;
					}
				}

				return -1;
			}

			void reset_textures(bool hard_reset=false)
			{
				if (hard_reset)
				{
					for (GLenum i = BASE_TEXTURE_INDEX; i < MAX_TEXTURES; i++)
					{
						unbind_texture(gl_texture_id);
					}
				}

				gl_texture_id = BASE_TEXTURE_INDEX;
			}
		protected:
			// Returns a reference to an internal buffer, populated with the indices requested.
			const std::vector<int>& get_texture_indices(const TextureArray& textures, std::size_t texture_count)
			{
				auto& buffer = _texture_assignment_buffer;

				// Clear the previous contents of the buffer.
				buffer.clear();

				for (const auto& texture : textures)
				{
					auto texture_index = get_texture_index(texture->get_handle());

					buffer.push_back(texture_index);
				}

				return buffer;
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
	Context::Context(app::Window& wnd, Backend gfx, Flags flags)
		: graphics_backend(gfx), state(memory::unique<Context::State>())
	{
		ASSERT(get_backend() == Backend::OpenGL);
		ASSERT(util::lib::establish_gl());

		native_context = new Driver(wnd.get_handle());

		glewInit();

		// Initial configuration:
		toggle(flags, true);

		// TODO: Implement this using context-flags:
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		//glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);
	}

	Context::~Context()
	{
		//ASSERT(get_backend() == Backend::OpenGL);

		delete reinterpret_cast<Driver*>(native_context);
	}

	Context::Flags Context::toggle(Flags flags, bool value)
	{
		return Driver::set_flags(*state, flags, value);
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

		if ((buffer_type & BufferType::Color))
		{
			buffer_flags |= GL_COLOR_BUFFER_BIT;
		}

		if (state->enabled(Flags::DepthTest))
		{
			if ((buffer_type & BufferType::Depth))
			{
				buffer_flags |= GL_DEPTH_BUFFER_BIT;
			}
		}

		if (buffer_flags == 0)
		{
			buffer_flags = GL_COLOR_BUFFER_BIT;
		}

		glClear(buffer_flags);
	}

	void Context::set_viewport(int x, int y, int width, int height)
	{
		glViewport(x, y, width, height);
	}

	void Context::draw()
	{
		const Mesh& mesh = state->mesh;

		draw(mesh.get_primitive());
	}

	void Context::draw(Primitive primitive)
	{
		const Mesh& mesh = state->mesh;

		const auto primitive_type = Driver::get_primitive(primitive);
		const auto offset = static_cast<GLint>(mesh.offset());
		const auto count = static_cast<GLsizei>(mesh.size());

		glDrawArrays(primitive_type, offset, count);
	}

	bool Context::set_uniform(Shader& shader, raw_string name, int value)
	{
		auto& driver = get_driver(*this);

		auto uniform = driver.get_uniform(shader, name);

		if (uniform == Driver::InvalidUniform)
		{
			return false;
		}

		glUniform1i(uniform, value);

		return true;
	}

	bool Context::set_uniform(Shader& shader, raw_string name, float value)
	{
		auto& driver = get_driver(*this);

		auto uniform = driver.get_uniform(shader, name);

		if (uniform == Driver::InvalidUniform)
		{
			return false;
		}

		glUniform1f(uniform, value);

		return true;
	}

	bool Context::set_uniform(Shader& shader, raw_string name, bool value)
	{
		return set_uniform(shader, name, static_cast<int>(value));
	}

	bool Context::set_uniform(Shader& shader, raw_string name, const math::Vector2D& value)
	{
		auto& driver = get_driver(*this);

		auto uniform = driver.get_uniform(shader, name);

		if (uniform == Driver::InvalidUniform)
		{
			return false;
		}

		glUniform2fv(uniform, 1, &value[0]);

		return true;
	}

	bool Context::set_uniform(Shader& shader, raw_string name, const math::Vector3D& value)
	{
		auto& driver = get_driver(*this);

		auto uniform = driver.get_uniform(shader, name);

		if (uniform == Driver::InvalidUniform)
		{
			return false;
		}

		glUniform3fv(uniform, 1, &value[0]);

		return true;
	}

	bool Context::set_uniform(Shader& shader, raw_string name, const math::Vector4D& value)
	{
		auto& driver = get_driver(*this);

		auto uniform = driver.get_uniform(shader, name);

		if (uniform == Driver::InvalidUniform)
		{
			return false;
		}

		glUniform4fv(uniform, 1, &value[0]);

		return true;
	}

	bool Context::set_uniform(Shader& shader, raw_string name, const math::Matrix2x2& value)
	{
		auto& driver = get_driver(*this);

		auto uniform = driver.get_uniform(shader, name);

		if (uniform == Driver::InvalidUniform)
		{
			return false;
		}

		glUniformMatrix2fv(uniform, 1, GL_FALSE, &value[0][0]);

		return true;
	}

	bool Context::set_uniform(Shader& shader, raw_string name, const math::Matrix3x3& value)
	{
		auto& driver = get_driver(*this);

		auto uniform = driver.get_uniform(shader, name);

		if (uniform == Driver::InvalidUniform)
		{
			return false;
		}

		glUniformMatrix3fv(uniform, 1, GL_FALSE, &value[0][0]);

		return true;
	}

	bool Context::set_uniform(Shader& shader, raw_string name, const math::Matrix4x4& value)
	{
		auto& driver = get_driver(*this);

		auto uniform = driver.get_uniform(shader, name);

		if (uniform == Driver::InvalidUniform)
		{
			return false;
		}

		glUniformMatrix4fv(uniform, 1, GL_FALSE, &value[0][0]);

		return true;
	}

	bool Context::set_uniform(Shader& shader, raw_string name, const TextureArray& textures)
	{
		auto& driver = get_driver(*this);

		auto uniform = driver.get_uniform(shader, name);

		if (uniform == Driver::InvalidUniform)
		{
			return false;
		}

		auto texture_count = textures.size();

		if (texture_count == 0)
		{
			return false;
		}
		else if (texture_count == 1)
		{
			return set_uniform(shader, name, textures[0]);
		}

		const auto& buffer = driver.get_texture_indices(textures, texture_count);

		glUniform1iv(uniform, texture_count, buffer.data());

		return true;
	}

	bool Context::set_uniform(Shader& shader, raw_string name, const pass_ref<Texture> texture)
	{
		return set_uniform(shader, name, *texture);
	}

	bool Context::set_uniform(Shader& shader, raw_string name, Texture& texture)
	{
		auto& driver = get_driver(*this);

		int texture_index = driver.get_texture_index(texture.get_handle());

		if (texture_index != -1)
		{
			return set_uniform(shader, name, texture_index);
		}

		return false;
	}

	bool Context::set_uniform(Shader& shader, const std::string& name, const UniformData& uniform)
	{
		bool result = false;

		std::visit([&](auto&& data)
		{
			result = set_uniform(shader, name, data);
		}, uniform);

		return result;
	}

	bool Context::apply_uniforms(Shader& shader, const UniformMap& uniforms)
	{
		for (const auto& uniform : uniforms)
		{
			const auto* name = uniform.first.c_str();
			const auto& data = uniform.second;

			if (!set_uniform(shader, name, data))
			{
				return false;
			}
		}

		return true;
	}

	// Force-flushes the uniforms stored by 'shader'.
	bool Context::flush_uniforms(Shader& shader)
	{
		return apply_uniforms(shader, shader.get_uniforms());
	}

	// Force-flushes the uniforms stored in the actively bound shader.
	bool Context::flush_uniforms()
	{
		return flush_uniforms(state->shader);
	}

	void Context::clear_textures(bool force)
	{
		auto& driver = get_driver(this);

		driver.reset_textures(force); // true;
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

		driver.push_texture(texture.handle);
		state->push_texture(texture);
		
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

	FrameBuffer& Context::bind(FrameBuffer& buffer, bool read_only)
	{
		// Retrieve the currently bound mesh.
		FrameBuffer& prev = state->framebuffer;

		const auto& native_rep = buffer.get_handle();

		const GLenum target = ((read_only) ? GL_READ_FRAMEBUFFER : GL_FRAMEBUFFER);

		glBindFramebuffer(target, native_rep);

		// Assign the newly bound mesh.
		state->framebuffer = buffer;

		// Return the previously bound mesh.
		return prev;
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
	Context::Handle Context::generate_texture(const PixelMap& texture_data, ElementType channel_type, TextureFlags flags, bool __keep_bound)
	{
		///ASSERT(texture_data);

		// At this time, no more than four color channels are supported.
		///ASSERT(texture_data.channels() <= 4);

		Handle texture = NoHandle;

		glGenTextures(1, &texture);

		// TODO: Add support for non-2D textures. (1D, 3D, etc)
		glBindTexture(GL_TEXTURE_2D, texture);

		// Apply texture flags:
		Driver::apply_texture_flags(flags);

		/*
			// TODO: Review behavior for Depth and Stencil buffers.
			bool is_depth_map = ((flags & TextureFlags::DepthMap));

			// For now, Depth maps are not supported.
			ASSERT(!is_depth_map);
		*/

		allocate_texture(texture_data.width(), texture_data.height(), texture_data.format(), channel_type, texture_data.data(), false); // true;

		if ((flags & TextureFlags::MipMap))
		{
			glGenerateMipmap(GL_TEXTURE_2D);
		}

		if (!__keep_bound)
		{
			glBindTexture(GL_TEXTURE_2D, 0);
		}

		return texture;
	}

	Context::Handle Context::generate_texture(int width, int height, TextureFormat format, ElementType element_type, TextureFlags flags, bool __keep_bound)
	{
		Handle texture = NoHandle;

		glGenTextures(1, &texture);

		// TODO: Add support for non-2D textures. (1D, 3D, etc)
		glBindTexture(GL_TEXTURE_2D, texture);

		// Apply texture flags.
		Driver::apply_texture_flags(flags);

		allocate_texture(width, height, format, element_type);

		if (!__keep_bound)
		{
			glBindTexture(GL_TEXTURE_2D, 0);
		}

		return texture;
	}

	void Context::allocate_texture(int width, int height, TextureFormat format, ElementType channel_type, const memory::raw_ptr raw_data, bool _calculate_exact_format)
	{
		const auto texture_format = Driver::get_texture_format(format, channel_type, _calculate_exact_format);
		const auto texture_layout = Driver::get_texture_layout(format);
		const auto element_type = Driver::get_element_type(channel_type);

		glTexImage2D(GL_TEXTURE_2D, 0, texture_format, width, height, 0, texture_layout, element_type, raw_data);
	}

	void Context::release_texture(Handle&& handle)
	{
		glDeleteTextures(1, &handle);
	}

	void Context::resize_texture(Texture& texture, int width, int height)
	{
		use(texture, [&]()
		{
			allocate_texture(width, height, texture.get_format(), texture.get_element_type(), nullptr);
		});

		texture.width = width;
		texture.height = height;
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

	// Framebuffer related:
	Context::Handle Context::generate_framebuffer()
	{
		GLuint buffer = NoHandle;

		glGenFramebuffers(1, &buffer);

		return buffer;
	}

	void Context::release_framebuffer(Context::Handle&& handle)
	{
		glDeleteFramebuffers(1, &handle);
	}

	bool Context::framebuffer_attachment(Texture& texture)
	{
		FrameBuffer& framebuffer = state->framebuffer;
		const auto attachment_index = framebuffer.get_attachment_index();

		const auto texture_handle = texture.get_handle();
		const auto buffer_handle = framebuffer.get_handle();

		const auto native_attachment_index = (GL_COLOR_ATTACHMENT0 + attachment_index);

		glFramebufferTexture2D(GL_FRAMEBUFFER, native_attachment_index, GL_TEXTURE_2D, texture_handle, 0);

		framebuffer.attachment_indices.push_back(native_attachment_index);
		framebuffer.attachments.push_back(texture);

		return true;
	}

	bool Context::framebuffer_link_attachments()
	{
		FrameBuffer& framebuffer = state->framebuffer;

		glDrawBuffers(framebuffer.attachment_indices.size(), framebuffer.attachment_indices.data());

		return true;
	}

	Context::Handle Context::generate_renderbuffer(RenderBufferType type, int width, int height)
	{
		if (type == RenderBufferType::Unknown)
		{
			return NoHandle;
		}

		TextureFormat format;
		ElementType element_type;

		switch (type)
		{
			case RenderBufferType::Color:
				format = TextureFormat::RGBA; // RGB;
				element_type = ElementType::UByte;

				break;
			case RenderBufferType::Stencil:
				format = TextureFormat::Stencil;
				element_type = ElementType::UByte;

				break;
			case RenderBufferType::Depth:
				format = TextureFormat::Depth;

				// TODO: Verify that this resolves correctly to 24-bit depth.
				// Driver-dependent depth bits.
				element_type = ElementType::Unknown; // Int24

				break;
			case RenderBufferType::DepthStencil:
				format = TextureFormat::DepthStencil;

				// 24-bit depth, 8-bit stencil.
				element_type = ElementType::UInt;
		}

		return generate_renderbuffer(type, width, height, format, element_type);
	}

	// Renderbuffer related:
	Context::Handle Context::generate_renderbuffer(RenderBufferType type, int width, int height, TextureFormat format, ElementType element_type)
	{
		if (type == RenderBufferType::Unknown)
		{
			return NoHandle;
		}

		if (!state->has_framebuffer())
		{
			return NoHandle;
		}

		Handle buffer = NoHandle;

		auto attachment_type = Driver::get_renderbuffer_attachment(type);
		auto texture_format = Driver::get_texture_format(format, element_type, true);
		
		glGenRenderbuffers(1, &buffer);
		glBindRenderbuffer(GL_RENDERBUFFER, buffer);
		glRenderbufferStorage(GL_RENDERBUFFER, texture_format, width, height);
		
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, attachment_type, GL_RENDERBUFFER, buffer);

		return buffer;
	}

	void Context::release_renderbuffer(Handle&& handle)
	{
		glDeleteRenderbuffers(1, &handle);
	}
}