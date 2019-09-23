#include "context.hpp"
#include "context_state.hpp"

#include "framebuffer.hpp"

namespace graphics
{
	// FrameBuffer:
	FrameBuffer::FrameBuffer(weak_ref<Context> ctx, ContextHandle&& handle)
		: Resource(ctx, std::move(handle)) {}

	FrameBuffer::FrameBuffer(pass_ref<Context> ctx)
		: FrameBuffer(ctx, ctx->generate_framebuffer()) {}

	FrameBuffer::~FrameBuffer()
	{
		// TODO: Implement framebuffer destruction.
		get_context()->release_framebuffer(std::move(handle));
	}

	bool FrameBuffer::attach(Texture& texture)
	{
		auto ctx = get_context();

		// Ensure that this framebuffer is appropriately bound.
		if (!ctx->state->bound(*this))
		{
			return false;
		}

		return ctx->framebuffer_attachment(texture);
	}

	// This method returns the next index used for texture attachment.
	unsigned int FrameBuffer::get_attachment_index() const
	{
		return static_cast<unsigned int>(attachments.size());
	}

	void swap(FrameBuffer& x, FrameBuffer& y)
	{
		swap(static_cast<Resource&>(x), static_cast<Resource&>(y));

		swap(x.attachments, y.attachments);
	}
}