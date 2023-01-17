#pragma once

// TODO: Deprecate.

#include <tuple>
#include <functional>
#include <utility>
#include <optional>
#include <type_traits>
#include <algorithm>

namespace engine
{
	template <typename fn_t, typename... argument_types>
	class Action
	{
		public:
			using function = fn_t;
			using arguments = std::tuple<argument_types...>;
		private:
			static_assert(!(std::is_rvalue_reference_v<argument_types>&&...));

			std::optional<function> f = std::nullopt;
			std::optional<arguments> args = std::nullopt;

			Action() = default;
			//Action(const Action&) = default;

			void exec()
			{
				if (args.has_value() && f.has_value())
				{
					std::apply(*f, *args);
				}
			}
		protected:
			//friend void swap(Action& x, Action& y);
		public:
			template
			<
				typename fwd_fn, typename... fwd_types,
				typename = std::enable_if_t<(std::is_convertible_v<fwd_types&&, argument_types> && ...)>
			>
			Action(fwd_fn&& func, fwd_types&&... args) noexcept:
				f(std::forward<fwd_fn>(func)),
				args({ std::forward<fwd_types>(args)... })
			{}

			Action& operator=(Action&& a) noexcept
			{
				using std::swap;

				swap(this->f, a.f);
				swap(this->args, a.args);

				return *this;
			}

			Action(Action&& a) noexcept : Action() { *this = std::move(a); }

			void operator()()
			{
				exec();

				f = std::nullopt;
				args = std::nullopt;
			}

			~Action()
			{
				exec();
			}
	};

	template <typename fn_t, typename... arguments>
	auto make_action(fn_t&& f, arguments&&... args)
	{
		return Action
		<
			std::decay_t<fn_t>,
			std::remove_cv_t<std::remove_reference_t<arguments>>...
		>
		(
			std::forward<fn_t>(f),
			std::forward<arguments>(args)...
		);
	}
}