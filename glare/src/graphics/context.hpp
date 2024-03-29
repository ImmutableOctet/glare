#pragma once

#include "types.hpp"
#include "context_flags.hpp"
#include "texture.hpp"
#include "texture_format.hpp"
#include "texture_array.hpp"
#include "framebuffer.hpp"
#include "buffer_type.hpp"
#include "buffer_access_mode.hpp"
#include "shader_type.hpp"
#include "primitive.hpp"
#include "vertex_attribute.hpp"
#include "vertex_winding.hpp"
#include "bind.hpp"
#include "viewport.hpp"
#include "array_types.hpp"
#include "uniform_data.hpp"
#include "uniform_map.hpp"
//#include "shader.hpp"

//#include "context_state.hpp"

// Driver-specific:
#include "drivers/drivers.hpp"

#include <util/memory.hpp>

#include <math/types.hpp>

#include <type_traits>
#include <utility>
#include <functional>
#include <string>
#include <string_view>
#include <tuple>
#include <optional>

//#include <version>
//#include <concepts>
#include <type_traits>

namespace app
{
	class Window;
}

namespace graphics
{
	class ContextState;
	class Canvas;

	// Static Resources:
	class ShaderSource;
	class PixelMap;

	// Dynamic Resources:
	class Shader;
	class Texture;
	class Mesh;
	class FrameBuffer;
	class RenderBuffer;

	enum class Backend
	{
		OpenGL   = 1,
		Direct3D = 2,
	};

	class Context
	{
		public:
			template <typename T>
			friend class ShaderVar;

			template <typename resource_t, typename bind_fn>
			friend class BindOperation;

			class Driver;

			// Friends:
			friend Driver;
			friend Canvas;
			friend Shader;
			friend Texture;
			friend FrameBuffer;
			friend RenderBuffer;
			friend Mesh;

			// Types:
			using Handle = ContextHandle;
			using Backend = graphics::Backend;

			using State = ContextState;
			using Flags = ContextFlags;

			// Constants:
			static constexpr Handle NoHandle = {};

			struct FrameBufferBindState
			{
				FrameBuffer* read_state = nullptr;
				FrameBuffer* write_state = nullptr;

				inline bool is_default() const { return ((read_state == nullptr) && is_readwrite()); }
				inline bool is_readwrite() const
				{
					if (read_state == write_state)
					{
						return true;
					}

					if (read_state != nullptr)
					{
						if (write_state != nullptr)
						{
							return (read_state->get_handle() == write_state->get_handle());
						}
					}

					return false;
				}
			};
		private:
			const Backend graphics_backend;

			// TODO: Graphics abstraction -- Use std::variant or similar.
			NativeContext native_context;

			std::unique_ptr<State> state;

			Viewport viewport;
		protected:
			// Bind Operations:

			// Binds the shader specified, returning
			// a reference to the previously bound shader.
			Shader& bind(Shader& shader);

			/*
				Binds the texture specified, returning
				a reference to the previously bound texture.
				
				NOTE: Texures bind to driver-controlled incremental indices.

				For this reason, each bind texture bind-operation
				results in a new allocation of a texture slot.
			*/
			const Texture& bind(const Texture& texture);
			const Texture& bind(const Texture& texture, const std::string& uniform_sampler); // std::string_view

			// Binds the mesh specified, returning
			// a reference to the previously bound mesh.
			Mesh& bind(Mesh& mesh);

			/* 
				Binds the framebuffer specified, returning
				a reference to the previously bound buffer.
				If 'read_only' is enabled, the buffer specified
				will only be bound for read operations.
			*/

			FrameBufferBindState bind(FrameBuffer& buffer, FrameBufferType bind_type=FrameBufferType::ReadWrite);
			FrameBufferBindState bind(FrameBufferBindState state, FrameBufferType _ignored_bind_type={});

			RenderBuffer& bind(RenderBuffer& texture);
		public:
			static std::tuple<TextureFormat, ElementType> resolve_renderbuffer_format(RenderBufferType type);

			Context(app::Window& wnd, Backend gfx, Flags flags=Flags::Default, bool extensions=false);
			~Context();

			inline graphics::Backend get_backend() const { return graphics_backend; }
			inline NativeContext get_native() { return native_context; }

			Flags set_flags(Flags flag, bool value);
			Flags get_flags() const;
			bool get_flag(Flags flag) const;

			// Commands:
			void flip(app::Window& wnd);

			void begin_frame();
			void end_frame();

			void clear(BufferType buffer_type=BufferType::ColorDepth);

			// TODO: Add an overload for vectors.
			void clear(float red, float green, float blue, float alpha, BufferType buffer_type=BufferType::ColorDepth);

			// Assigns the current rendering viewport.
			void set_viewport(const Viewport& viewport);

			inline Viewport get_viewport() const { return viewport; }

			// NOTE: Unsafe; use at your own risk.
			void clear_textures(bool force=false);

			// Creates a safe bind operation for the resource specified.
			template <typename ResourceType> // typename = typename std::enable_if<!std::is_const_v<ResourceType>>::type
			inline auto use(ResourceType& resource)
			{
				return bind_resource
				(
					*this, resource,

					[this](auto& res) -> auto&
					{
						return bind(res);
					}
				);
			}

			// Binds the resource specified, executes the function provided,
			// then automatically unbinds the resource appropirately.
			template <typename ResourceType, typename fn, typename = std::enable_if_t<std::is_invocable_v<fn>, fn>>
			inline void use(ResourceType& resource, fn exec)
			{
				/*
				auto op = bind_resource
				(
					*this, resource,

					[this](auto& res) -> auto& //decltype(res)
					{
						return bind(res);
					}
				);
				*/

				auto op = use(resource);

				exec();

				// ~op executes here.
			}

			inline auto use(const Texture& resource, const std::string& uniform_sampler) // std::string_view
			{
				return bind_resource
				(
					*this, resource,

					[&, this](const auto& res) -> const auto&
					{
						return bind(res, uniform_sampler);
					}
				);
			}

			inline auto use(const std::optional<Texture>& opt_texture, const std::string& uniform_sampler) // std::string_view
			//-> std::optional<decltype(use(*opt_texture, uniform_sampler))>
			{
				if (opt_texture.has_value())
				{
					return std::optional { use(*opt_texture, uniform_sampler) };
				}
				
				return std::optional<decltype(use(static_cast<const Texture&>(*opt_texture), uniform_sampler))> {};
				//return std::nullopt;
			}

			template <typename fn>
			inline void use(const Texture& resource, const std::string& uniform_sampler, fn exec) // std::string_view
			{
				auto op = use(resource, uniform_sampler);

				exec();

				// ~op executes here.
			}

			inline auto use(FrameBuffer& resource, FrameBufferType bind_type=FrameBufferType::ReadWrite)
			{
				return bind_resource
				(
					*this, resource,

					[&, this](auto& res) -> auto
					{
						return bind(res, bind_type);
					}
				);
			}

			template <typename fn>
			inline void use(FrameBuffer& resource, FrameBufferType bind_type, fn exec) // requires std::is_invocable<fn>
			{
				auto op = use(resource, bind_type);

				exec();

				// ~op executes here.
			}

			FrameBuffer& get_default_framebuffer();

			void copy_framebuffer(FrameBuffer& source, FrameBuffer& destination, const PointRect& src, const PointRect& dst, BufferType buffer_types=BufferType::Color, TextureFlags filter=TextureFlags::None);

			// Transfers pixel data between the currently bound read and write framebuffers. (See 'copy_framebuffer')
			void transfer_pixels(const PointRect& src, const PointRect& dst, BufferType buffer_types=BufferType::Color, TextureFlags filter=TextureFlags::None);

			inline void copy_framebuffer(FrameBuffer& source, const PointRect& src, const PointRect& dst, BufferType buffer_types, TextureFlags filter=TextureFlags::None)
			{
				copy_framebuffer(source, get_default_framebuffer(), src, dst, buffer_types, filter);
			}

			// Draw using the currently bound 'Mesh' object.
			void draw();

			// Draw using the currently bound 'Mesh' object, but with a different primitive type.
			void draw(Primitive primitive);

			bool set_uniform(Shader& shader, std::string_view name, std::int32_t value); // int
			//bool set_uniform(Shader& shader, std::string_view name, std::uint32_t value); // unsigned int
			bool set_uniform(Shader& shader, std::string_view name, std::int64_t value);
			bool set_uniform(Shader& shader, std::string_view name, std::uint64_t value);

			bool set_uniform(Shader& shader, std::string_view name, ContextHandle handle); // std::uint32_t

			bool set_uniform(Shader& shader, std::string_view name, std::uint8_t value); // std::uint32_t

			bool set_uniform(Shader& shader, std::string_view name, bool value);
			bool set_uniform(Shader& shader, std::string_view name, float value);

			bool set_uniform(Shader& shader, std::string_view name, const math::Vector2D& value);
			bool set_uniform(Shader& shader, std::string_view name, const math::Vector3D& value);
			bool set_uniform(Shader& shader, std::string_view name, const math::Vector4D& value);

			bool set_uniform(Shader& shader, std::string_view name, const graphics::VectorArray& values);
			bool set_uniform(Shader& shader, std::string_view name, const graphics::MatrixArray& values);
			bool set_uniform(Shader& shader, std::string_view name, const graphics::FloatArray& values);

			bool set_uniform(Shader& shader, std::string_view name, const math::Matrix2x2& value);
			bool set_uniform(Shader& shader, std::string_view name, const math::Matrix3x3& value);
			bool set_uniform(Shader& shader, std::string_view name, const math::Matrix4x4& value);


			bool set_uniform(Shader& shader, std::string_view name, const TextureArray& textures);
			bool set_uniform(Shader& shader, std::string_view name, const std::shared_ptr<Texture>& texture);
			bool set_uniform(Shader& shader, std::string_view name, const Texture& texture); // std::int32_t

			bool set_uniform(Shader& shader, std::string_view name, const UniformData& uniform);

			//bool set_uniform(Shader& shader, const std::string& name, const std::shared_ptr<Texture>& texture); // std::int32_t
			//bool set_uniform(Shader& shader, const std::string& name, const Texture& texture); // std::int32_t

			bool apply_uniforms(Shader& shader, const UniformMap& uniforms);

			// Force-flushes the uniforms stored by 'shader'.
			bool flush_uniforms(Shader& shader);

			// Force-flushes the uniforms stored in the actively bound shader.
			bool flush_uniforms();

			// Mesh related:

			// Generates a native mesh-composition. (Used internally by some types; e.g. `Mesh`)
			// TODO: Allow for more-detailed mesh descriptions. (Dynamic vs. static, etc.)
			MeshComposition generate_mesh
			(
				memory::memory_view vertices, std::size_t vertex_size,
				memory::array_view<VertexAttribute> attributes,
				memory::array_view<MeshIndex> indices=nullptr,
				BufferAccessMode access_mode=BufferAccessMode::StaticDraw
			) noexcept;

			void update_mesh
			(
				MeshComposition& mesh,
				memory::memory_view vertices, std::size_t vertex_size,
				memory::array_view<VertexAttribute> attributes,
				memory::array_view<MeshIndex> indices=nullptr,
				BufferAccessMode access_mode=BufferAccessMode::StaticDraw,
				bool reuse_buffer_allocation=false, bool update_attributes=true
			) noexcept;

			// Releases a native mesh-composition generated by `generate_mesh`.
			// (Used internally by some types; e.g. `Mesh`)
			void release_mesh(MeshComposition&& mesh) noexcept;
		protected:
			inline State& get_state() const { return *state; }

			void set_winding_order(VertexWinding winding_order);

			// Texture related:

			// TODO: Work-out mixed formats between input and native driver format.
			Handle generate_texture(const PixelMap& texture_data, ElementType channel_type, TextureFlags flags=TextureFlags::Default, TextureType type=TextureType::Default, bool _keep_bound=true, bool _loose_internal_format=true); // noexcept;
			Handle generate_texture(int width, int height, TextureFormat format, ElementType channel_type, TextureFlags flags=TextureFlags::Default, TextureType type=TextureType::Default, std::optional<ColorRGBA> _border_color=std::nullopt, bool _keep_bound=true, bool _loose_internal_format=true); // noexcept;

			void allocate_texture_2d(int width, int height, TextureFormat format, ElementType channel_type, const void* raw_data=nullptr, bool is_dynamic=true, bool generate_mipmaps=false, bool _calculate_exact_format=true, bool _loose_internal_format=true); // TextureType type
			void allocate_texture_cubemap(int width, int height, TextureFormat format, ElementType channel_type, const void* raw_data=nullptr, bool is_dynamic=true, bool generate_mipmaps=false, bool _calculate_exact_format=true); // TextureType type

			void release_texture(Handle&& handle);

			void resize_texture(Texture& texture, int width, int height, const void* raw_data=nullptr, bool generate_mipmaps=false);

			// Shader related:

			// Builds & links a shader program using the source code specified.
			Handle build_shader(const ShaderSource& source, std::optional<std::string_view> preprocessor=std::nullopt) noexcept; // generate_shader(...)

			// Links individual shader objects into one program. (see also: 'build_shader')
			Handle link_shader(const Handle& vertex_obj, const Handle& fragment_obj, const Handle& geometry_obj={}) noexcept;

			// Releases a built shader program.
			void release_shader(Handle&& handle);

			// Builds a native shader object of the type specified.
			// The string(s) passed to this function must be null-terminated
			Handle build_shader_source_obj(std::string_view source_text, ShaderType type, std::optional<std::string_view> preprocessor=std::nullopt, std::optional<std::string_view> version=std::nullopt) noexcept;

			// Releases a native shader object.
			void release_shader_source_obj(Handle&& handle);

			// Framebuffer related:
			Handle generate_framebuffer(); // noexcept;
			void release_framebuffer(Handle&& handle);

			// NOTE: 'framebuffer' must be bound prior to calling this method.
			// Attaches a texture to the currently bound framebuffer.
			// NOTE: The 'texture' reference must be non-const, as attaching it
			// to the currently bound framebuffer would indicate mutation.
			bool framebuffer_attachment(FrameBuffer& framebuffer, Texture& textur, bool force_attachment_data=false);
			
			// NOTE: 'framebuffer' must be bound prior to calling this method.
			// TODO: Look into alternatives to making this public.
			// Links the current framebuffer to its attachments.
			bool framebuffer_link_attachments(FrameBuffer& framebuffer);

			// Renderbuffer related:
			Handle generate_renderbuffer(RenderBufferType type, int width, int height);
			Handle generate_renderbuffer(RenderBufferType type, int width, int height, TextureFormat format, ElementType element_type);

			void resize_renderbuffer(RenderBuffer& renderbuffer, int width, int height);

			void release_renderbuffer(Handle&& handle);
	};
}