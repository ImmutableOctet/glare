#include <algorithm>

#include "resource.hpp"
#include "context.hpp"

namespace graphics
{
	Resource::Resource(weak_ref<Context> context, ContextHandle&& handle)
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

	void Resource::on_bind(Context& context) {} // Blank implementation.

	void swap(Resource& x, Resource& y)
	{
		using std::swap;

		swap(x.context, y.context);
		swap(x.handle, y.handle);
	}
}