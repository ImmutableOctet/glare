#pragma once

namespace engine
{
	// TODO: Determine if we should remove this outright.
	template <typename CoroutineType>
	class ForwardingAwaiter : public std::suspend_always
	{
		protected:
			using PromiseType = typename CoroutineType::promise_type;

			CoroutineType coroutine;

			std::coroutine_handle<PromiseType> parent_coroutine_handle = {};

		public:
			ForwardingAwaiter(ForwardingAwaiter&&) noexcept = default;
			ForwardingAwaiter(const ForwardingAwaiter&) = delete;

			ForwardingAwaiter(CoroutineType&& coroutine) noexcept :
				coroutine(std::move(coroutine))
			{}

			ForwardingAwaiter& operator=(ForwardingAwaiter&&) noexcept = default;
			ForwardingAwaiter& operator=(const ForwardingAwaiter&) = delete;

			ForwardingAwaiter& operator=(CoroutineType&& coroutine) noexcept
			{
				this->coroutine = std::move(coroutine);
			}

			decltype(auto) get_return_object() noexcept
			{
				return coroutine;
			}

			decltype(auto) get_coroutine_handle()
			{
				return coroutine.get_coroutine_handle();
			}

			decltype(auto) get_coroutine_handle() const
			{
				return coroutine.get_coroutine_handle();
			}

			decltype(auto) get_parent_coroutine_handle()
			{
				return parent_coroutine_handle;
			}

			decltype(auto) get_parent_coroutine_handle() const
			{
				return parent_coroutine_handle;
			}

			bool has_parent_coroutine_handle() const
			{
				return static_cast<bool>(parent_coroutine_handle);
			}

			decltype(auto) promise() const noexcept
			{
				return get_coroutine_handle().promise();
			}

			decltype(auto) parent_promise() const noexcept
			{
				return get_parent_coroutine_handle().promise();
			}

			decltype(auto) yield_value(auto&& value_ref) // noexcept
			{
				if constexpr
				(
					(traits::has_any_noexcept_permutation<traits::has_method_yield_value<PromiseType, std::suspend_always, decltype(value_ref)>, false>)
					||
					(traits::has_any_noexcept_permutation<traits::has_method_yield_value<PromiseType, std::suspend_never, decltype(value_ref)>, false>)
				)
				{
					auto&& initial_yield_result = promise().yield_value(std::forward<decltype(value_ref)>);

					forward_yield();

					return initial_yield_result;
				}
			}

			decltype(auto) yield_value() noexcept
			{
				return forward_yield();
			}

			auto forward_yield() noexcept
			{
				if (has_parent_coroutine_handle())
				{
					parent_promise().yield_value(coroutine);
				}

				return std::suspend_always {};
			}

			bool await_suspend(std::coroutine_handle<PromiseType> other_coroutine_handle) // noexcept
			{
				if (other_coroutine_handle.address() == get_coroutine_handle().address())
				{
					return false; // true;
				}

				parent_coroutine_handle = std::move(other_coroutine_handle);

				if (coroutine)
				{
					forward_yield();

					return false; // true;
				}
				
				return true; // false;
			}

			void await_resume()
			{
				if constexpr (traits::has_any_noexcept_permutation<traits::has_method_await_resume<PromiseType, void>>)
				{
					promise().await_resume();
				}
			}

			bool await_ready()
			{
				if constexpr (traits::has_any_noexcept_permutation<traits::has_method_await_ready<PromiseType, bool>, false>)
				{
					if (!promise().await_ready())
					{
						return false;
					}
				}
				else
				{
					if (!std::as_const(*this).await_ready())
					{
						return false;
					}
				}

				forward_yield();

				//return (!coroutine);
				return false;
			}

			bool await_ready() const
			{
				if constexpr (traits::has_any_noexcept_permutation<traits::has_method_await_ready<PromiseType, bool>, true>)
				{
					if (!promise().await_ready())
					{
						return false;
					}
				}

				//return (!coroutine);
				return false;
			}

			decltype(auto) initial_suspend()
			{
				if constexpr
				(
					(traits::has_any_noexcept_permutation<traits::has_method_initial_suspend<PromiseType, std::suspend_always>, false>)
					||
					(traits::has_any_noexcept_permutation<traits::has_method_initial_suspend<PromiseType, std::suspend_never>, false>)
				)
				{
					return promise().initial_suspend();
				}
				else
				{
					return std::as_const(*this).initial_suspend();
				}
			}

			decltype(auto) initial_suspend() const
			{
				if constexpr
				(
					(traits::has_any_noexcept_permutation<traits::has_method_initial_suspend<PromiseType, std::suspend_always>, true>)
					||
					(traits::has_any_noexcept_permutation<traits::has_method_initial_suspend<PromiseType, std::suspend_never>, true>)
				)
				{
					return promise().initial_suspend();
				}
				else
				{
					return std::suspend_always {};
				}
			}

			decltype(auto) final_suspend()
			{
				parent_coroutine_handle = {};

				if constexpr
				(
					(traits::has_any_noexcept_permutation<traits::has_method_final_suspend<PromiseType, std::suspend_always>, false>)
					||
					(traits::has_any_noexcept_permutation<traits::has_method_final_suspend<PromiseType, std::suspend_never>, false>)
				)
				{
					return promise().final_suspend();
				}
				else
				{
					return std::as_const(*this).final_suspend();
				}
			}

			decltype(auto) final_suspend() const
			{
				if constexpr
				(
					(traits::has_any_noexcept_permutation<traits::has_method_final_suspend<PromiseType, std::suspend_always>, true>)
					||
					(traits::has_any_noexcept_permutation<traits::has_method_final_suspend<PromiseType, std::suspend_never>, true>)
				)
				{
					return promise().final_suspend();
				}
				else
				{
					return std::suspend_always {};
				}
			}

			explicit operator bool() const
			{
				return await_ready();
			}
	};
}