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
	};
}