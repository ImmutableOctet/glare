#pragma once

#include "type_traits.hpp"
#include "member_traits.hpp"

#include <coroutine>
#include <exception>
#include <utility>
#include <type_traits>

#include <cassert>

namespace util
{
	template <typename T=void>
	class fiber;

	namespace impl
	{
		namespace fiber_traits
		{
			GENERATE_HAS_TYPE_MEMBER_TRAIT(value_type);
			GENERATE_HAS_TYPE_MEMBER_TRAIT(promise_type);
		}

		template <typename T>
		class fiber_promise
		{
			public:
				friend class fiber<T>;

				using value_type     = std::remove_reference_t<T>;

				using pointer_type   = value_type*;
				using reference_type = value_type&;

			private:
				template <typename T>
				inline static constexpr bool has_value_type_v = fiber_traits::has_type_member_value_type_v<T>;

				template <typename T>
				inline static constexpr bool has_promise_type_v = fiber_traits::has_type_member_promise_type_v<T>;

				template <typename PromiseType, bool restrict_to_impl=true>
				static constexpr bool is_compatible_promise_type_impl()
				{
					if constexpr ((!restrict_to_impl) || (util::is_specialization_v<PromiseType, util::impl::fiber_promise>))
					{
						// Alternative conditions:
						//
						// Based on underlying type of `value_ptr` (requires `value_ptr` to actually be a member):
						// if (std::is_convertible_v<decltype(std::declval<PromiseType>().value_ptr), pointer_type>)
						// 
						// Based on `pointer_type` (requires alias to be present):
						// if (std::is_convertible_v<PromiseType::pointer_type, pointer_type>)

						// Most generic option (only requires `value_type` alias as a visible member):
						if constexpr (has_value_type_v<PromiseType>)
						{
							if constexpr (std::is_convertible_v<typename PromiseType::value_type*, pointer_type>)
							{
								return true;
							}
						}
					}

					return false;
				}

				template <typename CoroutineType>
				static constexpr bool is_compatible_coroutine_type_impl()
				{
					if constexpr (has_promise_type_v<CoroutineType>)
					{
						return is_compatible_promise_v<typename CoroutineType::promise_type>;
					}

					return false;
				}

				template <typename FiberType>
				static constexpr bool is_compatible_fiber_type_impl()
				{
					if constexpr (std::is_same_v<FiberType, fiber<T>>)
					{
						return true;
					}
					else
					{
						if constexpr (is_fiber_v<FiberType>)
						{
							return is_compatible_coroutine_type_impl<FiberType>();
						}

						return false;
					}
				}

				pointer_type value_ptr = {};
				std::exception_ptr exception_ptr = {};
				fiber<T>* blocking_fiber_ptr = {};

			protected:
				using coroutine_handle_t = std::coroutine_handle<fiber_promise<T>>;

				// NOTE: `restrict_to_impl` forces usage of `util::impl::fiber_promise` specializations.
				template <typename PromiseType, bool restrict_to_impl=true>
				inline static constexpr bool is_compatible_promise_v = is_compatible_promise_type_impl<PromiseType, restrict_to_impl>();

				// NOTE: Use of this trait may be too restrictive if we want compatibility with other coroutine types later.
				template <typename FiberType>
				inline static constexpr bool is_fiber_v = util::is_specialization_v<FiberType, util::fiber>;

				// NOTE: Requires `promise_type` to be visible.
				template <typename FiberType>
				inline static constexpr bool is_compatible_fiber_v = is_compatible_fiber_type_impl<FiberType>();

				// See above notes about restrictions.
				template <typename CoroutineType>
				inline static constexpr bool is_compatible_coroutine_v = is_compatible_coroutine_type_impl<CoroutineType>(); // is_compatible_fiber_v<CoroutineType>;

				template <typename CoroutineType>
				std::suspend_always forward_yield(CoroutineType&& yield_ref) noexcept
				{
					if (yield_ref.resume())
					{
						if (!yield_ref.done())
						{
							if constexpr (!std::is_const_v<std::remove_reference_t<CoroutineType>>)
							{
								if constexpr (is_fiber_v<CoroutineType>)
								{
									set_blocker(yield_ref);
								}
							}

							return set_value_ptr(yield_ref.coroutine_handle.promise().value_ptr);
						}
					}

					clear_value();

					return {};
				}

				template <typename CoroutineType>
				std::suspend_always forward_return(CoroutineType&& return_ref) noexcept
				{
					return forward_yield(std::forward<CoroutineType>(return_ref));
				}

				// NOTE: We can't convert underlying values here, since we only store pointers.
				// 
				// If we wanted to support some kind of implicit conversion,
				// we would need to use some kind of type erasure under the hood. (e.g. an 'any' type)
				std::suspend_always set_value_ptr(pointer_type value_ptr) noexcept
				{
					this->value_ptr = value_ptr;

					return {};
				}

				bool set_blocker(fiber<T>& fiber)
				{
					if (blocking_fiber_ptr)
					{
						if (*blocking_fiber_ptr)
						{
							return false;
						}
					}

					if (fiber)
					{
						blocking_fiber_ptr = &fiber;

						return true;
					}
					else
					{
						clear_blocker();
					}

					return false;
				}

				void clear_blocker()
				{
					blocking_fiber_ptr = {};
				}

			public:
				fiber<T> get_return_object() noexcept
				{
					return { coroutine_handle_t::from_promise(*this) };
				}

				std::suspend_always initial_suspend() const noexcept { return {}; }

				// Always suspend to avoid early destruction.
				std::suspend_always final_suspend() const noexcept { return {}; }

				std::suspend_always yield_value(value_type&& value_ref) noexcept
				{
					// NOTE: Lifetime of `value_ref` tied to coroutine.
					// (See notes in templated overload)
					return set_value_ptr(static_cast<pointer_type>(&value_ref));
				}

				std::suspend_always yield_value(reference_type value_ref) noexcept
				{
					return set_value_ptr(static_cast<pointer_type>(&value_ref));
				}

				std::suspend_always yield_value(fiber<T>& yield_ref) noexcept
				{
					return forward_yield(yield_ref);
				}

				template
				<
					typename YieldType,

					typename=std::enable_if_t
					<
						(is_compatible_coroutine_v<YieldType>)
						||
						(is_compatible_promise_v<YieldType>)
					>
				>
				decltype(auto) yield_value(YieldType&& yield_ref) noexcept
				{
					if constexpr (is_compatible_coroutine_v<YieldType>)
					{
						return forward_yield(std::forward<YieldType>(yield_ref));
					}
					else // if constexpr (is_compatible_promise_v<YieldType>)
					{
						// NOTE: Lifetime of underlying must be preserved.
						return set_value_ptr(static_cast<pointer_type>(yield_ref.value_ptr));
					}
				}

				void unhandled_exception() noexcept
				{
					exception_ptr = std::current_exception();
				}

				void return_void() noexcept
				{
					clear_value();
				}

				void await_suspend(coroutine_handle_t) {}
				void await_resume() {}
				bool await_ready() { return true; }

				reference_type value() const noexcept
				{
					assert(value_ptr);

					// Enforce alignment.
					assert((reinterpret_cast<std::intptr_t>(value_ptr) % alignof(value_type)) == 0);

					return *value_ptr;
				}

				pointer_type try_value() const noexcept
				{
					return value_ptr;
				}

				void clear_value() noexcept
				{
					value_ptr = {};
				}

				void throw_if_exception() const
				{
					if (exception_ptr)
					{
						std::rethrow_exception(exception_ptr);
					}
				}

				fiber<T>* get_blocker() const
				{
					return blocking_fiber_ptr;
				}

				bool has_blocker() const
				{
					return static_cast<bool>(get_blocker());
				}
		};
	}

	template <>
	class fiber<void>
	{
		private:
			using coroutine_handle_t = std::coroutine_handle<void>;

			coroutine_handle_t coroutine_handle;

		public:
			constexpr fiber() noexcept = default;

			constexpr fiber(coroutine_handle_t coroutine_handle) noexcept :
				coroutine_handle(coroutine_handle)
			{}

			constexpr fiber(fiber&& other) noexcept :
				coroutine_handle(std::exchange(other.coroutine_handle, {}))
			{}

			constexpr ~fiber() noexcept
			{
				if (coroutine_handle)
				{
					coroutine_handle.destroy();
				}
			}

			constexpr fiber& operator=(fiber&& other) noexcept
			{
				std::swap(this->coroutine_handle, other.coroutine_handle);

				return *this;
			}

			constexpr fiber(const fiber&) = delete;
			constexpr fiber& operator=(const fiber&) = delete;

			constexpr bool exists() const
			{
				return static_cast<bool>(coroutine_handle);
			}

			bool can_continue() const noexcept
			{
				return ((coroutine_handle) && (!coroutine_handle.done()));
			}

			[[nodiscard]] bool done() const noexcept
			{
				return !can_continue();
			}

			void get()
			{
				if (can_continue())
				{
					coroutine_handle.resume();
				}
			}

			explicit operator bool() const noexcept
			{
				return can_continue();
			}

			bool operator==(const fiber&) const noexcept = default;
			bool operator!=(const fiber&) const noexcept = default;

			template <typename T>
			bool operator==(const fiber<T>& other) const noexcept
			{
				return (coroutine_handle.address() == other.coroutine_handle.address());
			}

			template <typename T>
			bool operator!=(const fiber<T>& other) const noexcept
			{
				return (!operator==(other));
			}

			void operator()()
			{
				get();
			}
	};

	template <typename T>
	class fiber
	{
		public:
			friend class impl::fiber_promise<T>;

			using promise_type = impl::fiber_promise<T>;
			using pointer_type = typename promise_type::pointer_type; // T*;

		protected:
			using coroutine_handle_t = std::coroutine_handle<promise_type>;

			coroutine_handle_t coroutine_handle = {};

		public:
			constexpr fiber() noexcept = default;

			constexpr fiber(coroutine_handle_t coroutine_handle) noexcept :
				coroutine_handle(coroutine_handle)
			{}

			constexpr fiber(fiber&& other) noexcept :
				coroutine_handle(std::exchange(other.coroutine_handle, {}))
			{}

			constexpr ~fiber() noexcept
			{
				if (coroutine_handle)
				{
					coroutine_handle.destroy();
				}
			}

			constexpr fiber(const fiber&) = delete;

			constexpr fiber& operator=(fiber&& other) noexcept
			{
				std::swap(this->coroutine_handle, other.coroutine_handle);

				return *this;
			}

			constexpr fiber& operator=(const fiber&) = delete;

			constexpr bool exists() const
			{
				return static_cast<bool>(coroutine_handle);
			}

			bool can_continue() const noexcept
			{
				return (exists() && (!coroutine_handle.done()));
			}

			[[nodiscard]] bool done() const noexcept
			{
				return (!can_continue());
			}

			pointer_type try_get()
			{
				if (resume())
				{
					if (!done())
					{
						auto value_out = try_get_latest_value();

						clear_latest_value();

						return value_out;
					}
				}

				return {};
			}

			decltype(auto) get()
			{
				if constexpr (!std::is_same_v<T, void>)
				{
					auto result = try_get();

					assert(result);

					return *result;
				}
				else
				{
					try_get();
				}
			}

			bool resume()
			{
				if (can_continue())
				{
					auto& coroutine_promise = promise();

					if (auto blocker = coroutine_promise.get_blocker())
					{
						if (blocker->resume())
						{
							return true;
						}
						else
						{
							coroutine_promise.clear_blocker();
						}
					}
					
					coroutine_handle.resume();

					if (coroutine_handle.done())
					{
						coroutine_promise.throw_if_exception();
					}
					else if (try_get_latest_value())
					{
						return true;
					}
					else
					{
						// Call resume again to finalize the coroutine.
						coroutine_handle.resume();
					}
				}

				return false;
			}

			constexpr bool has_value() const noexcept
			{
				return static_cast<bool>(coroutine_handle.promise().try_value());
			}

			pointer_type try_get_latest_value() const
			{
				return coroutine_handle.promise().try_value();
			}

			void clear_latest_value()
			{
				coroutine_handle.promise().clear_value();
			}

			bool await_ready() { return true; }
			void await_suspend(coroutine_handle_t) {}
			void await_resume() {}

			std::suspend_always initial_suspend() const noexcept { return {}; }
			std::suspend_always final_suspend() const noexcept { return {}; }

			decltype(auto) promise() const noexcept
			{
				return coroutine_handle.promise();
			}

			coroutine_handle_t& get_coroutine_handle()
			{
				return coroutine_handle;
			}

			const coroutine_handle_t& get_coroutine_handle() const
			{
				return coroutine_handle;
			}

			decltype(auto) get_blocker() const
			{
				return promise().get_blocker();
			}

			bool has_blocker() const
			{
				return promise().has_blocker();
			}

			explicit operator bool() const noexcept
			{
				return can_continue();
			}

			constexpr operator coroutine_handle_t&() &
			{
				return get_coroutine_handle();
			}

			constexpr operator const coroutine_handle_t&() const&
			{
				return get_coroutine_handle();
			}

			constexpr operator fiber<void>() && noexcept
			{
				return fiber<void> { std::move(coroutine_handle) };
			}

			decltype(auto) operator()()
			{
				return try_get();
			}

			promise_type operator co_await()
			{
				resume();

				return coroutine_handle.promise();
			}

			//bool operator==(const fiber&) const noexcept = default;
			//bool operator!=(const fiber&) const noexcept = default;

			bool operator==(const fiber& other) const noexcept
			{
				return (coroutine_handle.address() == other.coroutine_handle.address());
			}

			bool operator!=(const fiber& other) const noexcept
			{
				return (!operator==(other));
			}
	};

	using opaque_fiber = fiber<void>;
}