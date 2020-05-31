#pragma once

//#include <stack>
#include <vector>

#include "types.hpp"
#include "shader.hpp"
#include "framebuffer.hpp"
#include "texture.hpp"
#include "mesh.hpp"

namespace graphics
{
	class Context;
	class Canvas;

	// High-level state management:
	class ContextState
	{
		public:
			using Flags = ContextFlags;
		private:
			// TODO: Review behavior of 'textures' containers;
			// this value could be held as a reserved element of that container.
			Texture null_texture = {};
		protected:
			friend Context;
			friend Context::Driver;

			friend Canvas;

			// Modified and otherwise managed by 'Context' and 'Context::Driver'.
			Flags flags = Flags::Default;

			defaultable_ref<Shader> shader;
			defaultable_ref<FrameBuffer> framebuffer;
			defaultable_ref<Mesh> mesh;

			std::vector<const Texture*> textures; // std::stack<Texture>

			// There is no guarantee of the lifetime for the object returned by this method.
			const Texture& get_current_texture();
			
			void push_texture(const Texture& texture);
			const Texture& pop_texture();

			void clear_textures();
		public:
			inline Flags get_flags() const { return flags; }

			bool has_framebuffer() const;

			bool enabled(Flags check) const;

			// Returns 'true' if the resource specified is currently bound.
			bool bound(const Shader& shader) const;

			// Returns 'true' if the resource specified is currently bound.
			bool bound(const FrameBuffer& buffer) const;

			// Returns 'true' if the resource specified is currently bound.
			bool bound(const Mesh& mesh) const;

			// Returns 'true' if the resource specified is currently bound.
			bool bound(const Texture& texture) const;
		protected:
			void default_all();
	};
}