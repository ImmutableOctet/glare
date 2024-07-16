#pragma once

#include "script_traits.hpp"

#include <coroutine>
#include <type_traits>
#include <utility>

namespace engine
{
	class Script;

	template <typename RequestType, auto store_fn>
	class RequestAwaiter : public std::suspend_always
	{
		protected:
			using StoreFunctionType = decltype(store_fn);

			inline static constexpr bool has_get_script_method = engine::script::traits::has_method_get_script_v<RequestType, Script&>;
			inline static constexpr bool has_resume_type = (!std::is_same_v<typename RequestType::ResumeType, void>);

			void store()
			{
				// NOTE: We check `has_get_script_method` multiple times here to avoid nesting:
				if constexpr ((has_get_script_method) && (std::is_invocable_v<StoreFunctionType, Script&, decltype(*this)>))
				{
					auto& script = get_script();
						
					store_fn(script, *this);
				}
				else if constexpr ((has_get_script_method) && (std::is_invocable_v<StoreFunctionType, Script&>))
				{
					auto& script = get_script();

					store_fn(script);
				}
				else if constexpr (std::is_invocable_v<StoreFunctionType, RequestAwaiter&>)
				{
					store_fn(*this);
				}
			}

			RequestType request;

		public:
			RequestAwaiter(RequestAwaiter&&) noexcept = default;

			RequestAwaiter(RequestType&& request) noexcept :
				request(std::move(request))
			{}

			RequestAwaiter& operator=(RequestAwaiter&&) noexcept = default;

			RequestAwaiter& operator=(RequestType&& request) noexcept
			{
				this->request = std::move(request);
			}

			template <typename=std::enable_if_t<has_get_script_method>>
			decltype(auto) get_script()
			{
				return request.get_script();
			}

			bool await_ready() const
			{
				return request.get_boolean_result();
			}

			template <typename promise_type=void>
			bool await_suspend(std::coroutine_handle<promise_type> coroutine) // noexcept
			{
				store();

				if constexpr (has_resume_type)
				{
					if constexpr
					(
						(engine::script::traits::has_method_yield_value_v<promise_type, std::suspend_always, RequestType&>)
						||
						(engine::script::traits::has_method_yield_value_v<promise_type, std::suspend_never, RequestType&>)
					)
					{
						coroutine.promise().yield_value(request);
					}
				}
				else
				{
					using yield_value_type = typename promise_type::value_type;

					if constexpr (std::is_constructible_v<yield_value_type, RequestType&&>)
					{
						if constexpr
						(
							(engine::script::traits::has_method_yield_value_v<promise_type, std::suspend_always, yield_value_type>)
							||
							(engine::script::traits::has_method_yield_value_v<promise_type, std::suspend_never, yield_value_type>)
						)
						{
							// NOTE: Since we don't need to supply a value when `await_resume` is called (`co_await` finishes),
							// we can construct `yield_value_type` from `request` with permission to decay.
							coroutine.promise().yield_value(static_cast<yield_value_type>(std::move(request)));
						}
					}
				}

				return true;
			}

			decltype(auto) await_resume()
			{
				if constexpr (has_resume_type)
				{
					return static_cast<typename RequestType::ResumeType>(request);
				}
			}

			RequestType& get_request()
			{
				return request;
			}

			const RequestType& get_request() const
			{
				return request;
			}

			template <typename ...Args>
			decltype(auto) operator()(Args&&... args)
			{
				return request(std::forward<Args>(args)...);
			}

			template <typename ...Args>
			decltype(auto) operator()(Args&&... args) const
			{
				return request(std::forward<Args>(args)...);
			}

			operator RequestType&() &
			{
				return get_request();
			}

			operator const RequestType&() const&
			{
				return get_request();
			}

			explicit operator bool() const
			{
				return request();
			}
	};
}