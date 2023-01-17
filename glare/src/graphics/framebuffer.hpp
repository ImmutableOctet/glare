#pragma once

#include <vector>
#include <optional>
//#include <utility>

#include <debug.hpp>

#include "types.hpp"
#include "resource.hpp"
#include "renderbuffer.hpp"
#include "texture.hpp"

namespace graphics
{
	// Forward declarations:
	class ContextState;

	class FrameBuffer : public Resource
	{
		public:
			friend Context;
			friend ContextState;

			using Type = FrameBufferType;

			// The maximum number of attachments allowed by the underlying driver*
			static constexpr unsigned int MAX_ATTACHMENTS = 8; // 32;

			friend void swap(FrameBuffer& x, FrameBuffer& y);

			FrameBuffer();
			FrameBuffer(pass_ref<Context> ctx);
			FrameBuffer(const FrameBuffer&)=delete;

			FrameBuffer(FrameBuffer&& buffer) noexcept : FrameBuffer() { swap(*this, buffer); }

			~FrameBuffer();

			inline FrameBuffer& operator=(FrameBuffer buffer)
			{
				swap(*this, buffer);

				return *this;
			}

			// Attachments can only be altered when this framebuffer has been bound.
			// In the event attachment fails, this method will return 'false'.
			// NOTE: Attachments are handled by the underlying context, and are not enumerable from this interface.
			bool attach(Texture& texture);

			bool attach_depth(Texture& texture);

			bool attach(RenderBuffer&& buffer);
			bool attach(RenderBufferType type, int width, int height);

			bool link();
			
			bool resize(int width, int height);

			//bool detach_all();
		protected:
			// This method returns the next index used for texture attachment.
			unsigned int get_attachment_index() const;

			//void on_bind(Context& context) override;
			
			// TODO: Look into moving to 'shared_ptr' objects instead of raw references.
			// TODO: Review whether we want to keep this data available. (Required for 'detach', if added)
			// Attachments are handled by the associated context, but stored in the 'FrameBuffer' object itself.
			std::vector<defaultable_ref<Texture>> attachments;
			std::vector<unsigned int> attachment_indices;

			std::vector<RenderBuffer> render_buffers;
	};
}