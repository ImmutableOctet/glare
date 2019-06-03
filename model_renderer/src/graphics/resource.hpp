#pragma once

#pragma once

#include <debug.hpp>

#include "types.hpp"
#include "context.hpp"

namespace graphics
{
	class Resource
	{
		public:
			Resource(weak_ref<Context> context={}, Context::Handle&& handle={});

			inline Context::Handle get_handle() const { return handle; }
			inline ref<Context> get_context() const { return context.lock(); }

			bool operator==(const Resource& rhs) const;

			inline bool operator!=(const Resource& rhs) const
			{
				return !operator==(rhs);
			}
		protected:
			weak_ref<Context> context;
			Context::Handle handle = Context::NoHandle;
	};
}