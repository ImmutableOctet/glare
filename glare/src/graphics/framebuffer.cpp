#include <algorithm>

#include "context.hpp"
#include "context_state.hpp"

#include "framebuffer.hpp"

namespace graphics
{
	// FrameBuffer:
	FrameBuffer::FrameBuffer()
		: Resource({}, {}) {}

	FrameBuffer::FrameBuffer(pass_ref<Context> ctx)
		: Resource(ctx, ctx->generate_framebuffer()) {}

	FrameBuffer::~FrameBuffer()
	{
		auto ctx = get_context();

		if (!ctx)
		{
			return;
		}

		// TODO: Implement framebuffer destruction.
		ctx->release_framebuffer(std::move(handle));
	}

	bool FrameBuffer::attach(Texture& texture)
	{
		auto ctx = get_context();

		return ctx->framebuffer_attachment(*this, texture);
	}

	bool FrameBuffer::attach(RenderBufferType type, int width, int height)
	{
		auto ctx = get_context();

		// Ensure that this framebuffer is appropriately bound.
		if (!ctx->state->bound(*this))
		{
			return false;
		}

		return attach(RenderBuffer(ctx, type, width, height));
	}

	bool FrameBuffer::attach(RenderBuffer&& buffer)
	{
		auto ctx = get_context();

		// Ensure that this framebuffer is appropriately bound.
		if (!ctx->state->bound(*this))
		{
			return false;
		}

		render_buffers.push_back(std::move(buffer));

		return true;
	}

	bool FrameBuffer::link()
	{
		auto ctx = get_context();

		return ctx->framebuffer_link_attachments(*this);
	}

	bool FrameBuffer::resize(int width, int height)
	{
		auto ctx = get_context();

		// Ensure that this framebuffer is appropriately bound.
		/*
		if (!ctx->state->bound(*this))
		{
			return false;
		}
		*/

		ctx->use(*this, [&, this]()
		{
			for (auto& texture : attachments)
			{
				ctx->resize_texture(texture, width, height); // texture.resize(...)
			}

			for (auto& render_buffer : render_buffers)
			{
				ctx->resize_renderbuffer(render_buffer, width, height);
			}
		});

		return true;
	}

	// This method returns the next index used for texture attachment.
	unsigned int FrameBuffer::get_attachment_index() const
	{
		//return static_cast<unsigned int>(attachments.size());
		return static_cast<unsigned int>(attachment_indices.size());
	}

	void swap(FrameBuffer& x, FrameBuffer& y)
	{
		swap(static_cast<Resource&>(x), static_cast<Resource&>(y));

		swap(x.attachments, y.attachments);
		swap(x.attachment_indices, y.attachment_indices);
	}
}