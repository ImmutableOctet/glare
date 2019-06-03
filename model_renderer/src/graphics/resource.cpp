#include "resource.hpp"

namespace graphics
{
	Resource::Resource(weak_ref<Context> context, Context::Handle&& handle)
		: context(context), handle(handle) {}

	bool Resource::operator==(const Resource& rhs) const
	{
		if (rhs.get_handle() != get_handle())
		{
			return false;
		}

		if (rhs.get_context() != get_context())
		{
			return false;
		}

		return true;
	}
}