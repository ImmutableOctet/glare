#pragma once

#include <utility>
#include <functional>
//#include <iterator>

#include "types.hpp"
#include <math/math.hpp>

#include "bind.hpp"
//#include "context_state.hpp"

#include "vertex.hpp"

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
	class Shader;
	class ShaderSource;

	using NativeContext = void*;

	//using VertexAttributeIterator = std::iterator<std::input_iterator_tag, VertexAttribute>;

	enum Backend
	{
		OpenGL
	};

	class Context
	{
		public:
			// Friends:
			friend Shader;

			template <typename T>
			friend class ShaderVar;

			template <typename T>
			friend class Mesh;

			template <typename resource_t, typename bind_fn>
			friend class BindOperation;

			// Types:
			using Handle = ContextHandle;
			using Backend = graphics::Backend;

			using State = ContextState;

			// Constants:
			static constexpr Handle NoHandle = {};
		private:
			const Backend graphics_backend;

			// TODO: Graphics abstraction -- Use std::variant or similar.
			NativeContext native_context; // SDL_GLContext

			memory::unique_ref<State> state;
		protected:
			// Bind Operations:

			// Binds the shader specified, returning
			// a reference to the previously bound shader.
			Shader& bind(Shader& shader);
		public:
			Context(app::Window& wnd, Backend gfx);
			~Context();

			// Commands:
			void flip(app::Window& wnd);

			// TODO: Add an overload for vectors.
			void clear(float red, float green, float blue, float alpha);

			// Creates a safe bind operation for the resource specified.
			template <typename ResourceType>
			inline auto use(ResourceType& resource)
			{
				return bind_resource
				(
					resource,

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

			inline Backend get_backend() const { return graphics_backend; }
		protected:
			inline State& get_state() const { return *state; }
			
			// Mesh related:
			MeshComposition generate_mesh(memory::memory_view vertices, std::size_t vertex_size, memory::array_view<VertexAttribute> attributes, memory::array_view<MeshIndex> indices=nullptr) noexcept;

			void release_mesh(MeshComposition&& mesh);

			// Shader related:
			Handle build_shader(const ShaderSource& source); // generate_shader(...);
			void release_shader(Handle&& handle);

			bool set_uniform(Shader& shader, raw_string name, int value);
			bool set_uniform(Shader& shader, raw_string name, bool value);
			bool set_uniform(Shader& shader, raw_string name, float value);

			bool set_uniform(Shader& shader, raw_string name, const math::Vector2D& value);
			bool set_uniform(Shader& shader, raw_string name, const math::Vector3D& value);
			bool set_uniform(Shader& shader, raw_string name, const math::Vector4D& value);

			bool set_uniform(Shader& shader, raw_string name, const math::Matrix2x2& value);
			bool set_uniform(Shader& shader, raw_string name, const math::Matrix3x3& value);
			bool set_uniform(Shader& shader, raw_string name, const math::Matrix4x4& value);
	};
}