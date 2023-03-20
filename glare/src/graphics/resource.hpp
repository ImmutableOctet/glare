#pragma once

#include "types.hpp"

#include <memory>

namespace graphics
{
	class Context;

	class Resource
	{
		protected:
			template <typename resource_t, typename bind_fn>
			friend class BindOperation;

			// TODO: Review whether this should handle proper context-references instead.
			std::weak_ptr<Context> context;
			ContextHandle handle;
		public:
			inline ContextHandle get_handle() const { return handle; }
			inline std::shared_ptr<Context> get_context() const
			{
				auto ctx = context.lock();

				//ASSERT(ctx);

				return ctx;
			}

			bool operator==(const Resource& rhs) const;
			inline bool operator!=(const Resource& rhs) const { return !operator==(rhs); }

			virtual ~Resource();
		protected:
			friend void swap(Resource& x, Resource& y);

			Resource(std::weak_ptr<Context> context={}, ContextHandle&& handle={});
			Resource(const Resource&) = delete;

			inline Resource(Resource&& resource) noexcept : Resource() { swap(*this, resource); }

			inline Resource& operator=(Resource resource)
			{
				swap(*this, resource);

				return *this;
			}

			virtual void on_bind(Context& context) const;
	};
}