#pragma once

#include "types.hpp"
#include "resource.hpp"

#include "renderbuffer_type.hpp"

namespace graphics
{
	// Forward declarations:
	class Context;
	class FrameBuffer;

	// TODO: Review API safety of using 'RenderBuffer' directly.
	class RenderBuffer : public Resource
	{
		public:
			friend Context;
			friend FrameBuffer;

			using Type = RenderBufferType;

			friend void swap(RenderBuffer& x, RenderBuffer& y);
		protected:
			Type type;

			int width, height;

		public:
			RenderBuffer();
			RenderBuffer(const std::shared_ptr<Context>& ctx, Type type, int width, int height);
			RenderBuffer(RenderBuffer&& buffer) noexcept : RenderBuffer() { swap(*this, buffer); }

			~RenderBuffer();

			inline RenderBuffer& operator=(RenderBuffer buffer)
			{
				swap(*this, buffer);

				return *this;
			}

			inline Type get_type() const { return type; }

			inline int get_width() const { return width; }
			inline int get_height() const { return height; }

			inline PointRect rect() const // Viewport
			{
				return PointRect{ {}, { width, height } };
			}
	};
}