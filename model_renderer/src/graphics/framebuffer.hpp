#pragma once

#include <vector>
#include <utility>

#include <debug.hpp>

#include "types.hpp"
#include "resource.hpp"
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

			// The maximum number of attachments allowed by the underlying driver*
			static constexpr unsigned int MAX_ATTACHMENTS = 8; // 32;

			friend void swap(FrameBuffer& x, FrameBuffer& y);

			FrameBuffer(pass_ref<Context> ctx);

			inline FrameBuffer() : FrameBuffer({}, {}) {}

			FrameBuffer(const FrameBuffer&)=delete;

			~FrameBuffer();

			inline FrameBuffer& operator=(FrameBuffer buffer)
			{
				swap(*this, buffer);

				return *this;
			}

			// Attachments can only be altered when this framebuffer has been bound.
			// In the event attachment fails, this method will return 'false'.
			bool attach(Texture& texture);
			//void detach_all();
		protected:
			FrameBuffer(weak_ref<Context> ctx, ContextHandle&& resource_handle);

			// This method returns the next index used for texture attachment.
			unsigned int get_attachment_index() const;

			//void on_bind(Context& context) override;
			
			// TODO: Review whether we want to keep this data available. (Required for 'detach', if added)
			// Attachments are handled by the associated context, but stored in the 'FrameBuffer' object itself.
			std::vector<defaultable_ref<Texture>> attachments;
	};
}