#include "context_state.hpp"

namespace graphics
{
	Texture& ContextState::get_current_texture()
	{
		if (textures.size() > 0)
		{
			return *(textures.back());
		}

		return null_texture;
	}

	void ContextState::push_texture(Texture& texture)
	{
		textures.push_back(&texture);
	}

	Texture& ContextState::pop_texture()
	{
		if (textures.size() > 0)
		{
			textures.pop_back();
		}

		return get_current_texture();
	}

	void ContextState::clear_textures()
	{
		textures.clear();
	}

	void ContextState::default_all()
	{
		// Change every member to its default state:
		shader.make_default();
		mesh.make_default();

		clear_textures();
	}

	bool ContextState::enabled(Flags check) const
	{
		return (get_flags() & check);
	}
}