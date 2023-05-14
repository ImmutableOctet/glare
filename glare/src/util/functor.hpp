#pragma once

#include <utility>

namespace util
{
	/*
		A structure for converting a stateless `Functor` into a standalone function.

		Generates a static member-function called `execute` that will
		instantiate `Functor`, then immediately execute it with `Args`.
    
		The `execute` function will attempt to convert the returned value to `ReturnType`.
	*/
	template <typename Functor, typename ReturnType, typename... Args>
	struct wrap_functor
	{
		using decay_ptr_t = ReturnType(*)(Args...);

		static inline constexpr decay_ptr_t decay_ptr = Functor();

		static ReturnType execute(Args&&... args)
		{
			return static_cast<ReturnType>(Functor()(std::forward<Args>(args)...));
		}
	};
}