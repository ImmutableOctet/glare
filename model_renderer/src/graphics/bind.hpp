#pragma once

namespace graphics
{
	class Context;

	template <typename resource_t, typename bind_fn>
	class BindOperation
	{
		public:
			BindOperation(Context& context, resource_t& resource, bind_fn bind)
				: bind(bind), prev_resource(bind(resource))
			{
				resource.on_bind(context);
			}

			~BindOperation()
			{
				bind(prev_resource);
			}
		private:
			resource_t& prev_resource;
			bind_fn bind;
	};

	template <typename resource_t, typename bind_fn>
	auto bind_resource(Context& context, resource_t& resource, bind_fn bind)
	{
		return BindOperation<resource_t, bind_fn>(context, resource, bind);
	}
}