#pragma once

#include <utility>
#include <functional>
#include <string>

#include <math/math.hpp>

#include "types.hpp"
#include "bind.hpp"
#include "uniform.hpp"

//#include "context_state.hpp"

// Driver-specific:
#include "drivers/drivers.hpp"

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

	enum Backend
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
			friend Shader;
			friend Texture;
			friend FrameBuffer;
			friend Mesh;

			// Types:
			using Handle = ContextHandle;
			using Backend = graphics::Backend;

			using State = ContextState;
			using Flags = ContextFlags;

			// Constants:
			static constexpr Handle NoHandle = {};
		private:
			const Backend graphics_backend;

			// TODO: Graphics abstraction -- Use std::variant or similar.
			NativeContext native_context;

			memory::unique_ref<State> state;
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
			Texture& bind(Texture& texture);

			// Binds the mesh specified, returning
			// a reference to the previously bound mesh.
			Mesh& bind(Mesh& mesh);

			/* 
				Binds the framebuffer specified, returning
				a reference to the previously bound buffer.
				If 'read_only' is enabled, the buffer specified
				will only be bound for read operations.
			*/

			FrameBuffer& bind(FrameBuffer& buffer, bool read_only=false);
		public:
			Context(app::Window& wnd, Backend gfx, Flags flags=Flags::Default);
			~Context();

			inline Backend get_backend() const { return graphics_backend; }
			inline NativeContext get_native() { return native_context; }

			Flags toggle(Flags flag, bool value);

			// Commands:
			void flip(app::Window& wnd);

			// TODO: Add an overload for vectors.
			void clear(float red, float green, float blue, float alpha, BufferType buffer_type=BufferType::ColorDepth);

			// Assigns the current rendering viewport.
			void set_viewport(int x, int y, int width, int height);

			// NOTE: Unsafe; use at your own risk.
			void clear_textures();

			// Creates a safe bind operation for the resource specified.
			template <typename ResourceType>
			inline auto use(ResourceType& resource)
			{
				return bind_resource
				(
					*this, resource,

					[this](ResourceType& res) -> ResourceType&
					{
						return bind(res);
					}
				);
			}

			// Binds the resource specified, executes the function provided,
			// then automatically unbinds the resource appropirately.
			template <typename ResourceType, typename fn>
			inline void use(ResourceType& resource, fn exec)
			{
				auto op = use(resource);

				exec();
			}

			// Draw using the currently bound 'Mesh' object.
			void draw();

			// Draw using the currently bound 'Mesh' object, but with a different primitive type.
			void draw(Primitive primitive);

			bool set_uniform(Shader& shader, raw_string name, int value);
			bool set_uniform(Shader& shader, raw_string name, bool value);
			bool set_uniform(Shader& shader, raw_string name, float value);

			bool set_uniform(Shader& shader, raw_string name, const math::Vector2D& value);
			bool set_uniform(Shader& shader, raw_string name, const math::Vector3D& value);
			bool set_uniform(Shader& shader, raw_string name, const math::Vector4D& value);

			bool set_uniform(Shader& shader, raw_string name, const math::Matrix2x2& value);
			bool set_uniform(Shader& shader, raw_string name, const math::Matrix3x3& value);
			bool set_uniform(Shader& shader, raw_string name, const math::Matrix4x4& value);

			template <typename T>
			inline bool update(Shader& shader, const Uniform<T>& uniform)
			{
				return set_uniform(shader, uniform.get_name_raw(), uniform.get_value());
			}
		protected:
			inline State& get_state() const { return *state; }
			
			// Mesh related:
			MeshComposition generate_mesh(memory::memory_view vertices, std::size_t vertex_size, memory::array_view<VertexAttribute> attributes, memory::array_view<MeshIndex> indices=nullptr) noexcept;
			void release_mesh(MeshComposition&& mesh);

			// Texture related:

			// TODO: Work-out mixed formats between input and native driver format.
			Handle generate_texture(const PixelMap& texture_data, ElementType channel_type, TextureFlags flags=TextureFlags::Default, bool __keep_bound=true); // noexcept;
			Handle generate_texture(int width, int height, TextureFormat format, ElementType channel_type, TextureFlags flags=TextureFlags::Default, bool __keep_bound=true); // noexcept;

			void allocate_texture(int width, int height, TextureFormat texture_format, ElementType channel_type, const memory::raw_ptr raw_data=nullptr, bool _calculate_exact_format=true);

			void release_texture(Handle&& handle);

			// Shader related:
			Handle build_shader(const ShaderSource& source) noexcept; // generate_shader(...)
			void release_shader(Handle&& handle);

			// Framebuffer related:
			Handle generate_framebuffer(); // noexcept;
			void release_framebuffer(Handle&& handle);

			// Attaches a texture to the currently bound framebuffer.
			// NOTE: The 'texture' reference must be non-const, as attaching it
			// to the currently bound framebuffer would indicate mutation.
			bool framebuffer_attachment(Texture& texture);
	};
}