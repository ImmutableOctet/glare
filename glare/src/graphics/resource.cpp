#include <algorithm>

#include "resource.hpp"
#include "context.hpp"

namespace graphics
{
	Resource::Resource(std::weak_ptr<Context> context, ContextHandle&& handle)
		: context(context), handle(handle) {}

	Resource::~Resource() {}

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

	void Resource::on_bind(Context& context) const {} // Blank implementation.

	void swap(Resource& x, Resource& y)
	{
		using std::swap;

		swap(x.context, y.context);
		swap(x.handle, y.handle);
	}
}