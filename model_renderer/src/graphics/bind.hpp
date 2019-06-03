#pragma once

namespace graphics
{
	template <typename resource_t, typename bind_fn>
	class BindOperation
	{
		public:
			BindOperation(resource_t& resource, bind_fn bind)
				: bind(bind), prev_resource(bind(resource)) {}

			~BindOperation()
			{
				bind(prev_resource);
			}
		private:
			resource_t& prev_resource;
			bind_fn bind;
	};

	template <typename resource_t, typename bind_fn>
	auto bind_resource(resource_t& resource, bind_fn bind)
	{
		return BindOperation<resource_t, bind_fn>(resource, bind);
	}
}