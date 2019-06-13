#pragma once

//#include <stack>
#include <vector>

#include "types.hpp"
#include "shader.hpp"
#include "texture.hpp"
#include "mesh.hpp"

namespace graphics
{
	class Context;

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

			// Modified and otherwise managed by 'Context' and 'Context::Driver'.
			Flags flags = Flags::Default;

			defaultable_ref<Shader> shader;
			defaultable_ref<Mesh> mesh;

			std::vector<Texture*> textures; // std::stack<Texture>

			// There is no guarantee of the lifetime for the object returned by this method.
			Texture& get_current_texture();
			
			void push_texture(Texture& texture);
			Texture& pop_texture();

			void clear_textures();
		public:
			inline Flags get_flags() const { return flags; }

			bool enabled(Flags check) const;
		protected:
			void default_all();
	};
}