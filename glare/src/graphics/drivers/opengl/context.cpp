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

//#include "gl_driver.hpp"

#include <glm/gtc/type_ptr.hpp>

#include <sdl2/SDL_video.h>

#include <utility>
#include <variant>
#include <vector>
#include <unordered_map>
//#include <stack>
#include <type_traits>
#include <tuple>

// Debugging related:
#include <iostream>

namespace graphics
{
	using Driver = Context::Driver;

	// Default framebuffer object; meant to always represent 'NoHandle'.
	static FrameBuffer _main_framebuffer;

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
			static constexpr GLenum MAX_FB_COLOR_ATTACHMENTS = 16;
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

			UniformLocationContainer uniform_location_cache;
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
			static GLbitfield gl_get_buffer_flags(BufferType buffer_type, bool default_to_color_buffer=false, bool allow_depth=true)
			{
				GLbitfield buffer_flags = 0; // {};

				if ((buffer_type & BufferType::Color))
				{
					buffer_flags |= GL_COLOR_BUFFER_BIT;
				}

				if (allow_depth)
				{
					if ((buffer_type & BufferType::Depth))
					{
						buffer_flags |= GL_DEPTH_BUFFER_BIT;
					}
				}

				if (default_to_color_buffer)
				{
					if (buffer_flags == 0)
					{
						buffer_flags = GL_COLOR_BUFFER_BIT;
					}
				}

				return buffer_flags;
			}

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

			// NOTE: 'name' must be a null-terminated string.
			gl_uniform_location get_uniform(Shader& shader, std::string_view name)
			{
				auto& uniform_location_map = uniform_location_cache[shader.get_handle()];

				auto it = uniform_location_map.find(name);

				if (it != uniform_location_map.end())
				{
					return it->second;
				}

				auto uniform_location = glGetUniformLocation(shader.get_handle(), name.data());

				if (uniform_location == -1)
				{
					//std::cout << "Unable to resolve uniform for: " << '\"' << name << '\"' << '\n';
				}

				uniform_location_map[name] = uniform_location;

				return uniform_location;
			}

			// Applies texture flags to the currently bound texture.
			static void apply_texture_flags(TextureFlags flags, TextureType type=TextureType::Texture2D)
			{
				const auto texture_type = get_texture_type(type);

				if ((flags & TextureFlags::Clamp))
				{
					glTexParameteri(texture_type, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
					glTexParameteri(texture_type, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
					glTexParameteri(texture_type, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
				}
				else
				{
					// Wrapping parameters:
					if ((flags & TextureFlags::WrapS))
					{
						glTexParameteri(texture_type, GL_TEXTURE_WRAP_S, GL_REPEAT);
					}

					if ((flags & TextureFlags::WrapT))
					{
						glTexParameteri(texture_type, GL_TEXTURE_WRAP_T, GL_REPEAT);
					}
				}

				// TODO: Review filtering behavior.

				// Filtering parameters:
				if ((flags & TextureFlags::LinearFiltering))
				{
					if ((flags & TextureFlags::MipMap))
					{
						glTexParameteri(texture_type, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
						glTexParameteri(texture_type, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
					}
					else
					{
						glTexParameteri(texture_type, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
						glTexParameteri(texture_type, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
					}
				}
				else
				{
					if ((flags & TextureFlags::MipMap))
					{
						glTexParameteri(texture_type, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
						glTexParameteri(texture_type, GL_TEXTURE_MAG_FILTER, GL_NEAREST_MIPMAP_NEAREST);
					}
					else
					{
						glTexParameteri(texture_type, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
						glTexParameteri(texture_type, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
					}
				}
			}

			static GLenum get_texture_type(TextureType type)
			{
				switch (type)
				{
					case TextureType::Texture2D:
						return GL_TEXTURE_2D;
					case TextureType::CubeMap:
						return GL_TEXTURE_CUBE_MAP;
				}

				return GL_NONE;
			}

			// Returns the native OpenGL equivalent of 'ElementType'.
			static GLenum get_element_type(ElementType type, bool _allow_type_half=false, bool _allow_type_double=true) // true
			{
				switch (type)
				{
					case ElementType::Byte:    return GL_BYTE;
					case ElementType::UByte:   return GL_UNSIGNED_BYTE;
					case ElementType::Short:   return GL_SHORT;
					case ElementType::UShort:  return GL_UNSIGNED_SHORT;
					case ElementType::Int:     return GL_INT;
					case ElementType::UInt:    return GL_UNSIGNED_INT;
					
					case ElementType::Half:    if (_allow_type_half) return GL_HALF_FLOAT;
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
								case ElementType::Byte:
								case ElementType::UByte:
									return GL_RGB8;
								
								case ElementType::Short:
								case ElementType::UShort:
									return GL_RGB16;
								
								case ElementType::Int:
								case ElementType::UInt:
									return GL_RGB32I;

								case ElementType::Half:
									return GL_RGB16F;
								case ElementType::Float:
									return GL_RGB32F;
							}

							break;
						case GL_RGBA:
							switch (element_type)
							{
								case ElementType::Byte:
								case ElementType::UByte:
									return GL_RGBA8;
								
								case ElementType::Short:
								case ElementType::UShort:
									return GL_RGBA16;
								
								case ElementType::Int:
								case ElementType::UInt:
									return GL_RGBA32I;
								
								case ElementType::Half:
									return GL_RGBA16F;
								
								case ElementType::Float:
									return GL_RGBA32F;
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

			static GLenum get_framebuffer_attachment(TextureFormat type)
			{
				switch (type)
				{
					case TextureFormat::Depth:
						return GL_DEPTH_ATTACHMENT;
					case TextureFormat::Stencil:
						return GL_STENCIL_ATTACHMENT;
					case TextureFormat::DepthStencil:
						return GL_DEPTH_STENCIL_ATTACHMENT;

					/*
					case TextureFormat::RGB:
						return (GL_COLOR_ATTACHMENT0 + index);
					*/
				}

				return (GL_COLOR_ATTACHMENT0); //  + index // GL_NONE;
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

			static constexpr GLenum get_shader_type(ShaderType type) // noexcept
			{
				switch (type)
				{
					case ShaderType::Vertex:
						return GL_VERTEX_SHADER;
					case ShaderType::Fragment:
						return GL_FRAGMENT_SHADER;
					case ShaderType::Geometry:
						return GL_GEOMETRY_SHADER;
					//case ShaderType::Tessellation:
					//	return GL_GEOMETRY_SHADER;
				}

				return GL_NONE;
			}

			static Context::Handle compile_shader(const std::string& source, GLenum type) noexcept
			{
				if (source.empty())
				{
					return {};
				}

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

				bool swap_interval = (static_cast<int>(flags & Flags::VSync) > 0);

				SDL_GL_SetSwapInterval(swap_interval);

				// Depth-testing:
				if ((flags & Flags::DepthTest))
				{
					glDepthFunc(GL_LESS);
				}

				// Face culling:
				handle_flag(state, flags, Flags::FaceCulling, GL_CULL_FACE, value);

				if ((flags & Flags::FaceCulling))
				{
					// Discard back-faces, keep front-faces.
					//glCullFace(GL_FRONT_AND_BACK); // GL_FRONT // GL_BACK
					//glCullFace(GL_FRONT);

					// Default behavior:
					glFrontFace(GL_CW); // VertexWinding::Clockwise
					glCullFace(GL_BACK);
				}

				// Debugging related:
				/*
				//glFrontFace(GL_CCW);
				glDisable(GL_CULL_FACE);
				glDisable(GL_DEPTH_TEST);
				*/

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
			int get_texture_index(Handle texture) const
			{
				for (std::size_t i = 0; i < texture_stack.size(); i++) // auto
				{
					if (texture_stack[i] == texture)
					{
						return i;
					}
				}

				return -1;
			}

			GLenum get_active_texture_id() const
			{
				return (gl_texture_id - 1);
			}

			int get_active_texture_index() const
			{
				auto id = get_active_texture_id();

				return static_cast<int>((id - BASE_TEXTURE_INDEX));
			}

			void reset_textures(bool hard_reset=false)
			{
				if (hard_reset)
				{
					for (GLenum i = BASE_TEXTURE_INDEX; i < (BASE_TEXTURE_INDEX + MAX_TEXTURES); i++)
					{
						unbind_texture(gl_texture_id);
					}
				}

				gl_texture_id = BASE_TEXTURE_INDEX;
			}
			
			void bind_shader(Handle shader)
			{
				// Bind the shader to the current graphical context.
				glUseProgram(shader);

				//uniform_location_map = uniform_location_cache[shader];
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

		//glEnable(GL_DEPTH_TEST);


		// TODO: Implement this using context-flags:

		// Default configuration:
		/*
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		*/
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

	void Context::clear(BufferType buffer_type)
	{
		GLbitfield buffer_flags = Driver::gl_get_buffer_flags
		(
			buffer_type,

			true, state->enabled(Flags::DepthTest)
		);

		glClear(buffer_flags);
	}

	void Context::clear(float red, float green, float blue, float alpha, BufferType buffer_type)
	{
		// TODO: Look into clear color state. (Likely to be added to a 'Canvas' type)
		glClearColor(red, green, blue, alpha);

		clear(buffer_type);
	}

	void Context::set_viewport(int x, int y, int width, int height)
	{
		glViewport(x, y, width, height);
	}

	FrameBuffer& Context::get_default_framebuffer()
	{
		return _main_framebuffer;
	}

	void Context::copy_framebuffer(FrameBuffer& source, FrameBuffer& destination, const PointRect& src, const PointRect& dst, BufferType buffer_types, TextureFlags filter)
	{
		use(source, FrameBufferType::Read, [&]()
		{
			use(destination, FrameBufferType::Write, [&]()
			{
				transfer_pixels(src, dst, buffer_types, filter);
			});
		});
	}

	void Context::transfer_pixels(const PointRect& src, const PointRect& dst, BufferType buffer_types, TextureFlags filter)
	{
		auto gl_buffer_flags = Driver::gl_get_buffer_flags(buffer_types);

		GLenum gl_filter = GL_NEAREST;

		if ((filter & TextureFlags::LinearFiltering))
		{
			gl_filter = GL_LINEAR;
		}

		glBlitFramebuffer(src.start.x, src.start.y, src.end.x, src.end.y, dst.start.x, dst.start.y, dst.end.x, dst.end.y, gl_buffer_flags, gl_filter);
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

	bool Context::set_uniform(Shader& shader, std::string_view name, int value)
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

	bool Context::set_uniform(Shader& shader, std::string_view name, float value)
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

	bool Context::set_uniform(Shader& shader, std::string_view name, ContextHandle handle)
	{
		return set_uniform(shader, name, static_cast<int>(handle));
	}

	bool Context::set_uniform(Shader& shader, std::string_view name, bool value)
	{
		return set_uniform(shader, name, static_cast<int>(value));
	}

	bool Context::set_uniform(Shader& shader, std::string_view name, const math::Vector2D& value)
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

	bool Context::set_uniform(Shader& shader, std::string_view name, const math::Vector3D& value)
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

	bool Context::set_uniform(Shader& shader, std::string_view name, const math::Vector4D& value)
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

	bool Context::set_uniform(Shader& shader, std::string_view name, const math::Matrix2x2& value)
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

	bool Context::set_uniform(Shader& shader, std::string_view name, const math::Matrix3x3& value)
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

	bool Context::set_uniform(Shader& shader, std::string_view name, const math::Matrix4x4& value)
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

	bool Context::set_uniform(Shader& shader, std::string_view name, const TextureArray& textures)
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

	bool Context::set_uniform(Shader& shader, std::string_view name, const pass_ref<Texture> texture)
	{
		return set_uniform(shader, name, *texture);
	}

	bool Context::set_uniform(Shader& shader, std::string_view name, const Texture& texture)
	{
		auto& driver = get_driver(*this);

		int texture_index = driver.get_texture_index(texture.get_handle());

		if (texture_index != -1)
		{
			return set_uniform(shader, name, texture_index);
		}

		return false;
	}

	bool Context::set_uniform(Shader& shader, std::string_view name, const UniformData& uniform)
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
			const auto& name = uniform.first;
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

		auto& driver = get_driver(this);

		driver.bind_shader(shader.get_handle());

		// Assign the newly bound shader.
		state->shader = shader;

		// Return the previously bound shader.
		return prev_shader;
	}

	const Texture& Context::bind(const Texture& texture)
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

	const Texture& Context::bind(const Texture& texture, const std::string& uniform_sampler) // std::string_view
	{
		auto& driver = get_driver(this);
		auto& res = bind(texture);

		auto bound_idx = driver.get_active_texture_index();

		if (!uniform_sampler.empty())
		{
			static_cast<Shader&>(state->shader)[uniform_sampler] = bound_idx; // texture.get_handle();
		}

		return res;
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

	Context::FrameBufferBindState Context::bind(FrameBuffer& buffer, FrameBufferType bind_type)
	{
		GLenum target = 0; // (GL_FRAMEBUFFER);

		if ((bind_type == FrameBufferType::ReadWrite))
		{
			target = GL_FRAMEBUFFER;
		}
		else if ((bind_type & FrameBufferType::Read))
		{
			target = GL_READ_FRAMEBUFFER;
		}
		else if ((bind_type & FrameBufferType::Write))
		{
			target = GL_DRAW_FRAMEBUFFER;
		}

		const auto& native_rep = buffer.get_handle();

		glBindFramebuffer(target, native_rep);

		FrameBuffer* prev_read = nullptr;
		FrameBuffer* prev_write = nullptr;

		if ((bind_type & FrameBufferType::Read))
		{
			prev_read = &(state->get_read_framebuffer());
			state->read_framebuffer = buffer;
		}

		if ((bind_type & FrameBufferType::Write))
		{
			prev_write = &(state->get_write_framebuffer());
			state->write_framebuffer = buffer;
		}

		return { prev_read, prev_write };
	}

	Context::FrameBufferBindState Context::bind(FrameBufferBindState state, FrameBufferType _ignored_bind_type) // FrameBufferType _ignored_bind_type={}
	{
		if (state.is_readwrite()) // If both are the same (Includes 'nullptr' scenario)
		{
			FrameBuffer& buffer = ((state.read_state == nullptr) ? (get_default_framebuffer()) : (*state.read_state));

			return bind(buffer, FrameBufferType::ReadWrite);
		}
		else if (state.read_state != nullptr)
		{
			return bind(*state.read_state, FrameBufferType::Read);
		}
		else if (state.write_state != nullptr)
		{
			return bind(*state.write_state, FrameBufferType::Write);
		}

		return { nullptr, nullptr }; // {};
	}

	RenderBuffer& Context::bind(RenderBuffer& renderbuffer)
	{
		// Retrieve the currently bound render-buffer.
		RenderBuffer& prev_renderbuffer = state->renderbuffer;

		glBindRenderbuffer(GL_RENDERBUFFER, renderbuffer.get_handle());

		// Assign the newly bound mesh.
		state->renderbuffer = renderbuffer;

		// Return the previously bound mesh.
		return prev_renderbuffer;
	}

	std::tuple<TextureFormat, ElementType> Context::resolve_renderbuffer_format(RenderBufferType type)
	{
		TextureFormat format     = TextureFormat::RGBA; // {};
		ElementType element_type = ElementType::UByte; // {};

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

				break;
		}

		return { format, element_type };
	}

	// Mesh related:
	MeshComposition Context::generate_mesh(memory::memory_view vertices, std::size_t vertex_size, memory::array_view<VertexAttribute> attributes, memory::array_view<MeshIndex> indices) noexcept
	{
		ASSERT(vertices);
		ASSERT(vertex_size);
		ASSERT(attributes);

		GLComposition mesh = {};

		const void* vertex_data_offset = 0;
		const bool indices_available = (indices.has_data());

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

	// TODO: Implement cubemaps, etc. for this overload.
	Context::Handle Context::generate_texture(const PixelMap& texture_data, ElementType channel_type, TextureFlags flags, TextureType type, bool __keep_bound)
	{
		///ASSERT(texture_data);

		// At this time, no more than four color channels are supported.
		///ASSERT(texture_data.channels() <= 4);

		Handle texture = NoHandle;

		const auto texture_type = Driver::get_texture_type(type);

		glGenTextures(1, &texture);

		// TODO: Add support for non-2D textures. (1D, 3D, etc)
		glBindTexture(texture_type, texture);

		/*
			// TODO: Review behavior for Depth and Stencil buffers.
			bool is_depth_map = ((flags & TextureFlags::DepthMap));

			// For now, Depth maps are not supported.
			ASSERT(!is_depth_map);
		*/

		allocate_texture_2d(texture_data.width(), texture_data.height(), texture_data.format(), channel_type, texture_data.data(), (flags & TextureFlags::Dynamic), (flags & TextureFlags::MipMap), false); // true; // texture_type

		// Apply texture flags:
		Driver::apply_texture_flags(flags, type);

		if (!__keep_bound)
		{
			glBindTexture(texture_type, 0);
		}

		return texture;
	}

	Context::Handle Context::generate_texture(int width, int height, TextureFormat format, ElementType element_type, TextureFlags flags, TextureType type, std::optional<ColorRGBA> _border_color, bool __keep_bound)
	{
		Handle texture = NoHandle;

		const auto texture_type = Driver::get_texture_type(type);

		glGenTextures(1, &texture);

		// TODO: Add support for non-2D textures. (1D, 3D, etc)
		glBindTexture(texture_type, texture);

		switch (type)
		{
			case TextureType::Texture2D:
				allocate_texture_2d(width, height, format, element_type, nullptr, true, false); // texture_type

				break;
			case TextureType::CubeMap:
				allocate_texture_cubemap(width, height, format, element_type, nullptr, true, false);

				break;
		}

		// Apply texture flags.
		Driver::apply_texture_flags(flags, type);

		if (_border_color.has_value())
		{
			glTexParameterfv(texture_type, GL_TEXTURE_BORDER_COLOR, glm::value_ptr(*_border_color));
		}

		if (!__keep_bound)
		{
			glBindTexture(texture_type, 0);
		}

		return texture;
	}

	void Context::allocate_texture_2d(int width, int height, TextureFormat format, ElementType channel_type, const memory::raw_ptr raw_data, bool is_dynamic, bool generate_mipmaps, bool _calculate_exact_format) // TextureType type
	{
		const GLenum texture_type = GL_TEXTURE_2D;
		//const auto texture_type = Driver::get_texture_type(type);

		const auto texture_format = Driver::get_texture_format(format, channel_type, ((_calculate_exact_format) || (!is_dynamic)));
		const auto element_type   = Driver::get_element_type(channel_type);
		const auto texture_layout = Driver::get_texture_layout(format);

		if (is_dynamic)
		{
			glTexImage2D(texture_type, 0, texture_format, width, height, 0, texture_layout, element_type, raw_data); // TODO: Add option to specify mipmap levels.
		}
		else
		{
			glTexStorage2D(texture_type, 1, texture_format, width, height); // TODO: Add option to specify mipmap levels.
			glTexSubImage2D(texture_type, 0, 0, 0, width, height, texture_layout, element_type, raw_data);
		}

		if (generate_mipmaps)
		{
			glGenerateMipmap(texture_type);
		}
	}

	// TODO: Determine if this can be implemented using 'glTexStorage2D' and co. (see: 'is_dynamic' argument for 'allocate_texture_2d')
	void Context::allocate_texture_cubemap(int width, int height, TextureFormat format, ElementType channel_type, const memory::raw_ptr raw_data, bool is_dynamic, bool generate_mipmaps, bool _calculate_exact_format)
	{
		const auto texture_format = Driver::get_texture_format(format, channel_type, (_calculate_exact_format)); // ((_calculate_exact_format) || (!is_dynamic))
		const auto element_type = Driver::get_element_type(channel_type); // element_type
		const auto texture_layout = Driver::get_texture_layout(format);

		for (GLenum i = 0; i < 6; ++i) // unsigned int
		{
			const auto texture_type = (GL_TEXTURE_CUBE_MAP_POSITIVE_X + i);

			glTexImage2D(texture_type, 0, texture_format, width, height, 0, texture_layout, element_type, raw_data);

			// TODO: Confirm that this works for cubemaps.
			if (generate_mipmaps)
			{
				glGenerateMipmap(texture_type);
			}
		}
	}

	void Context::release_texture(Handle&& handle)
	{
		glDeleteTextures(1, &handle);
	}

	void Context::resize_texture(Texture& texture, int width, int height, const memory::raw_ptr raw_data, bool generate_mipmaps)
	{
		ASSERT(texture.is_dynamic());

		use(texture, [&]()
		{
			allocate_texture_2d(width, height, texture.get_format(), texture.get_element_type(), raw_data, true, generate_mipmaps);
		});

		texture.width = width;
		texture.height = height;
	}

	// Shader related:
	Context::Handle Context::build_shader(const ShaderSource& source) noexcept
	{
		auto vertex   = build_shader_source_obj(source.vertex, ShaderType::Vertex);
		auto fragment = build_shader_source_obj(source.fragment, ShaderType::Fragment);
		auto geometry = build_shader_source_obj(source.geometry, ShaderType::Geometry);

		auto program = link_shader(vertex, fragment, geometry);

		release_shader_source_obj(std::move(vertex));
		release_shader_source_obj(std::move(fragment));
		release_shader_source_obj(std::move(geometry));

		return program;
	}

	Context::Handle Context::link_shader(const Handle& vertex_obj, const Handle& fragment_obj, const Handle& geometry_obj) noexcept
	{
		// TODO: Look into proper error handling if possible.
		ASSERT(vertex_obj);
		ASSERT(fragment_obj);

		// Allocate a shader-program object.
		auto program = glCreateProgram();

		// Attach our compiled shader objects.
		glAttachShader(program, vertex_obj);
		glAttachShader(program, fragment_obj);

		if (geometry_obj != NoHandle)
		{
			glAttachShader(program, geometry_obj);
		}

		// Link together the program.
		glLinkProgram(program);

		GLint success;
		glGetProgramiv(program, GL_LINK_STATUS, &success);

		// TODO: Look into proper error handling for link-failure if possible.
		ASSERT(success);

		return program;
	}

	void Context::release_shader(Context::Handle&& handle)
	{
		glDeleteProgram(handle);
	}

	Context::Handle Context::build_shader_source_obj(const ShaderSource::StringType& source_text, ShaderType type) noexcept
	{
		return Driver::compile_shader(source_text, Driver::get_shader_type(type));
	}

	void Context::release_shader_source_obj(Handle&& handle)
	{
		// 'NoHandle' is a valid input for this function, whereas it may not be for OpenGL.
		if (handle != NoHandle)
		{
			glDeleteShader(handle);
		}
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
		if (handle == Context::NoHandle)
		{
			return;
		}

		glDeleteFramebuffers(1, &handle);
	}

	bool Context::framebuffer_attachment(FrameBuffer& framebuffer, Texture& texture, bool force_attachment_data)
	{
		// Ensure that this framebuffer is appropriately bound.
		//if (!state->bound(framebuffer))
		if (state->get_write_framebuffer().get_handle() != framebuffer.get_handle())
		{
			return false;
		}

		const auto attachment_index = framebuffer.get_attachment_index();

		//FrameBuffer& framebuffer = state->framebuffer;
		const auto texture_handle = texture.get_handle();
		const auto buffer_handle = framebuffer.get_handle();
		const auto texture_type = Driver::get_texture_type(texture.type); // GL_TEXTURE_2D

		auto native_attachment_index = Driver::get_framebuffer_attachment(texture.format); // GL_COLOR_ATTACHMENT0

		bool is_color_attachment = (native_attachment_index == GL_COLOR_ATTACHMENT0);

		if (is_color_attachment)
		{
			native_attachment_index += (attachment_index % Driver::MAX_FB_COLOR_ATTACHMENTS);
		}

		switch (texture.type)
		{
			case TextureType::Texture2D:

				glFramebufferTexture2D(GL_FRAMEBUFFER, native_attachment_index, texture_type, texture_handle, 0);

				break;
			case TextureType::CubeMap:
				glFramebufferTexture(GL_FRAMEBUFFER, native_attachment_index, texture_handle, 0);

				break;
		}

		if ((is_color_attachment) || force_attachment_data)
		{
			framebuffer.attachment_indices.push_back(native_attachment_index);
			framebuffer.attachments.push_back(texture);
		}

		return true;
	}

	bool Context::framebuffer_link_attachments(FrameBuffer& framebuffer)
	{
		//FrameBuffer& framebuffer = state->framebuffer;

		// Ensure that this framebuffer is appropriately bound.
		//if (!state->bound(framebuffer))
		if (state->get_write_framebuffer().get_handle() != framebuffer.get_handle())
		{
			return false;
		}

		if (framebuffer.attachment_indices.size() > 0)
		{
			glDrawBuffers(framebuffer.attachment_indices.size(), framebuffer.attachment_indices.data());
		}
		else
		{
			glDrawBuffer(GL_NONE);
			glReadBuffer(GL_NONE);
		}

		return true;
	}

	Context::Handle Context::generate_renderbuffer(RenderBufferType type, int width, int height)
	{
		if (type == RenderBufferType::Unknown)
		{
			return NoHandle;
		}

		auto [format, element_type] = resolve_renderbuffer_format(type);

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

		//if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		//	std::cout << "Framebuffer not complete!" << std::endl;

		return buffer;
	}

	void Context::resize_renderbuffer(RenderBuffer& renderbuffer, int width, int height)
	{
		use(renderbuffer, [&]()
		{
			auto [format, element_type] = resolve_renderbuffer_format(renderbuffer.get_type());
			auto texture_format = Driver::get_texture_format(format, element_type, true);

			glRenderbufferStorage(GL_RENDERBUFFER, texture_format, width, height);
		});

		renderbuffer.width = width;
		renderbuffer.height = height;
	}

	void Context::release_renderbuffer(Handle&& handle)
	{
		glDeleteRenderbuffers(1, &handle);
	}
}