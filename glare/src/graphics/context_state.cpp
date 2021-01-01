#include "context_state.hpp"

namespace graphics
{
	const Texture& ContextState::get_current_texture()
	{
		if (textures.size() > 0)
		{
			return *(textures.back());
		}

		return null_texture;
	}

	void ContextState::push_texture(const Texture& texture)
	{
		textures.push_back(&texture);
	}

	const Texture& ContextState::pop_texture()
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

		read_framebuffer.make_default();
		write_framebuffer.make_default();

		clear_textures();
	}

	bool ContextState::enabled(Flags check) const
	{
		return (get_flags() & check);
	}

	FrameBuffer& ContextState::get_read_framebuffer()
	{
		return read_framebuffer;
	}

	FrameBuffer& ContextState::get_write_framebuffer()
	{
		return write_framebuffer;
	}

	bool ContextState::has_read_framebuffer() const
	{
		return (!read_framebuffer.is_default());
	}

	bool ContextState::has_write_framebuffer() const
	{
		return (!write_framebuffer.is_default());
	}

	bool ContextState::has_framebuffer() const
	{
		return
		(has_read_framebuffer())
		||
		(has_write_framebuffer());
	}

	bool ContextState::bound(const Shader& shader) const
	{
		return ((&(this->shader)) == (&shader));
	}

	bool ContextState::bound(const FrameBuffer& buffer) const
	{
		return
		((&(this->read_framebuffer)) == (&buffer))
		||
		((&(this->write_framebuffer)) == (&buffer));
	}

	bool ContextState::bound(const Mesh& mesh) const
	{
		return ((&(this->mesh)) == (&mesh));
	}

	bool ContextState::bound(const Texture& texture) const
	{
		for (auto* t_ptr : this->textures)
		{
			if (t_ptr == (&texture))
			{
				return true;
			}
		}

		return false;
	}
}