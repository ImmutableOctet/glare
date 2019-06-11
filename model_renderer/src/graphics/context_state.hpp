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
		private:
			// TODO: Review behavior of 'textures' containers;
			// this value could be held as a reserved element of that container.
			Texture null_texture = {};
		protected:
			friend Context;

			// The currently bound shader.
			defaultable_ref<Shader> shader;
			defaultable_ref<Mesh> mesh;

			std::vector<Texture*> textures; // std::stack<Texture>

			// There is no guarantee of the lifetime for the object returned by this method.
			Texture& get_current_texture()
			{
				if (textures.size() > 0)
				{
					return *(textures.back());
				}

				return null_texture;
			}

			void push_texture(Texture& texture)
			{
				textures.push_back(&texture);
			}

			Texture& pop_texture()
			{
				if (textures.size() > 0)
				{
					textures.pop_back();
				}

				return get_current_texture();
			}

			void clear_textures()
			{
				textures.clear();
			}
		protected:
			void default_all()
			{
				// Change every member to its default state:
				shader.make_default();
				mesh.make_default();
				
				clear_textures();
			}
	};
}