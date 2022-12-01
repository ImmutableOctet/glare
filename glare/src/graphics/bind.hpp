#pragma once

#include <type_traits>

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

			BindOperation(BindOperation&&) noexcept = default;
			BindOperation(const BindOperation&) = delete;

			BindOperation& operator=(BindOperation&&) noexcept = default;
			BindOperation& operator=(const BindOperation&) = delete;

			~BindOperation()
			{
				bind(prev_resource);
			}
		private:
			//resource_t& prev_resource;
			std::invoke_result_t<bind_fn, resource_t&> prev_resource;
			bind_fn bind;
	};

	template <typename resource_t, typename bind_fn>
	auto bind_resource(Context& context, resource_t& resource, bind_fn bind)
	{
		return BindOperation(context, resource, bind);
	}
}