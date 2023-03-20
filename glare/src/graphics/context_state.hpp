#pragma once

//#include <stack>
#include <vector>
//#include <utility>

#include "types.hpp"
#include "shader.hpp"
#include "framebuffer.hpp"
#include "renderbuffer.hpp"
#include "texture.hpp"
#include "mesh.hpp"

#include <util/defaultable_ref.hpp>

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

			util::defaultable_ref<Shader> shader;
			util::defaultable_ref<Mesh> mesh;

			util::defaultable_ref<FrameBuffer> read_framebuffer;
			util::defaultable_ref<FrameBuffer> write_framebuffer;

			util::defaultable_ref<RenderBuffer> renderbuffer;

			std::vector<const Texture*> textures; // std::stack<Texture>

			// There is no guarantee of the lifetime for the object returned by this method.
			const Texture& get_current_texture();
			
			void push_texture(const Texture& texture);
			const Texture& pop_texture();

			void clear_textures();
		public:
			inline Flags get_flags() const { return flags; }

			FrameBuffer& get_read_framebuffer();
			FrameBuffer& get_write_framebuffer();

			bool has_read_framebuffer() const;
			bool has_write_framebuffer() const;
			bool has_framebuffer() const;
			//bool has_renderbuffer() const;

			bool enabled(Flags check) const;
			bool disabled(Flags check) const { return !enabled(check); }

			// Returns 'true' if the resource specified is currently bound.
			bool bound(const Shader& shader) const;

			// Returns 'true' if the resource specified is currently bound.
			bool bound(const FrameBuffer& buffer) const;

			// Returns 'true' if the resource specified is currently bound.
			bool bound(const RenderBuffer& buffer) const;

			// Returns 'true' if the resource specified is currently bound.
			bool bound(const Mesh& mesh) const;

			// Returns 'true' if the resource specified is currently bound.
			bool bound(const Texture& texture) const;
		protected:
			void set_flag_value(Flags flag, bool value);

			void default_all();
	};
}