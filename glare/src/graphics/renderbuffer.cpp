#include <algorithm>

#include "context.hpp"
#include "renderbuffer.hpp"

namespace graphics
{
	// RenderBuffer:
	RenderBuffer::RenderBuffer()
		: RenderBuffer({}, RenderBufferType::Unknown, 0, 0) {}

	RenderBuffer::RenderBuffer(pass_ref<Context> ctx, Type type, int width, int height)
		: Resource(ctx, ctx->generate_renderbuffer(type, width, height)), type(type), width(width), height(height)
	{
	}

	RenderBuffer::~RenderBuffer()
	{
		auto ctx = get_context();

		if (!ctx)
		{
			return;
		}

		// TODO: Implement framebuffer destruction.
		ctx->release_renderbuffer(std::move(handle));
	}

	void swap(RenderBuffer& x, RenderBuffer& y)
	{
		using std::swap;

		swap(static_cast<Resource&>(x), static_cast<Resource&>(y));

		swap(x.type, y.type);

		swap(x.width, y.width);
		swap(x.height, y.height);
	}
}