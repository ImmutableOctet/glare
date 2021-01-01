#pragma once

#pragma once

#include <debug.hpp>

#include "types.hpp"

namespace graphics
{
	class Context;

	class Resource
	{
		protected:
			template <typename resource_t, typename bind_fn>
			friend class BindOperation;

			// TODO: Review whether this should handle proper context-references instead.
			weak_ref<Context> context;
			ContextHandle handle;
		public:
			inline ContextHandle get_handle() const { return handle; }
			inline ref<Context> get_context() const { return context.lock(); }

			bool operator==(const Resource& rhs) const;
			inline bool operator!=(const Resource& rhs) const { return !operator==(rhs); }
		protected:
			friend void swap(Resource& x, Resource& y);

			Resource(weak_ref<Context> context={}, ContextHandle&& handle={});
			Resource(const Resource&) = delete;

			inline Resource(Resource&& resource) : Resource() { swap(*this, resource); }

			inline Resource& operator=(Resource resource)
			{
				swap(*this, resource);

				return *this;
			}

			virtual void on_bind(Context& context) const;
	};
}