#pragma once

#include "script_interface.hpp"
#include "yield_request.hpp"
#include "predicate.hpp"

#include "runtime_impl.hpp"

namespace engine
{
	class Script;

	template <typename RequestType, typename PredicateType, auto predicate_wrapper_fn, bool always_generate_continuation_predicate=false>
	class ConditionalYieldRequestImpl : public YieldRequestImpl<RequestType>
	{
		protected:
			using PredicateWrapperType = decltype(predicate_wrapper_fn(std::declval<PredicateType>()));

		public:
			using Base = ConditionalYieldRequestImpl::YieldRequestImpl;

			ConditionalYieldRequestImpl(RequestType yield_request, PredicateType predicate) :
				Base(std::move(yield_request)),
				script(engine::impl::get_running_script()),
				predicate(predicate_wrapper_fn(std::move(predicate)))
			{}

			ConditionalYieldRequestImpl(Script& script, RequestType yield_request, PredicateType predicate) :
				Base(std::move(yield_request)),
				script(&script),
				predicate(predicate_wrapper_fn(std::move(predicate)))
			{}

			bool has_script() const
			{
				return static_cast<bool>(script);
			}

			bool set_script(Script& script_to_bind)
			{
				if (has_script())
				{
					return false;
				}

				this->script = &script_to_bind;

				return true;
			}

			Script& get_script()
			{
				assert(script);

				return *script;
			}

			const Script& get_script() const
			{
				assert(script);

				return *script;
			}

			ScriptInterface& get_script_interface()
			{
				return ScriptInterface::get(get_script());
			}

			const ScriptInterface& get_script_interface() const
			{
				return ScriptInterface::get(get_script());
			}

			bool get_boolean_result() const override
			{
				return result;
			}

			//using Base::operator();

		protected:
			RequestType decay() override
			{
				if constexpr (std::is_same_v<std::remove_cvref_t<PredicateWrapperType>, std::nullptr_t>)
				{
					get_script().set_yield_continuation_predicate(nullptr);
				}
				else if constexpr (std::is_invocable_r_v<bool, PredicateWrapperType, const Script&, const MetaAny&>)
				{
					get_script().set_yield_continuation_predicate(std::move(predicate));
				}
				else if constexpr (always_generate_continuation_predicate)
				{
					get_script().set_yield_continuation_predicate(impl::generate_continuation_predicate_wrapper<RequestType>(std::move(predicate)));
				}

				return Base::decay();
			}

			bool set_result(bool value)
			{
				result = value;

				return value;
			}

			Script* script = nullptr;
			PredicateWrapperType predicate;

			bool result = false;
	};

	template <typename RequestType, typename PredicateType>
	class ConditionalYieldRequest : public ConditionalYieldRequestImpl<RequestType, PredicateType, &impl::generate_as_is_predicate_wrapper<PredicateType>, true>
	{
		public:
			using Base = ConditionalYieldRequest::ConditionalYieldRequestImpl;

			ConditionalYieldRequest(Script& script, RequestType yield_request, PredicateType predicate) :
				Base(script, std::move(yield_request), std::move(predicate))
			{}

			ConditionalYieldRequest(RequestType yield_request, PredicateType predicate) :
				Base(std::move(yield_request), std::move(predicate))
			{}

			/*
			bool operator()() const override
			{
				return Base::operator()();
			}
			*/

			bool operator()() override
			{
				if constexpr (std::is_invocable_r_v<bool, Base::PredicateWrapperType, decltype(*this), const RequestType&>)
				{
					return this->set_result(this->predicate(*this, this->get_request()));
				}
				else if constexpr (std::is_invocable_r_v<bool, Base::PredicateWrapperType, decltype(*this)>)
				{
					return this->set_result(this->predicate(*this));
				}
				else if constexpr (std::is_invocable_r_v<bool, Base::PredicateWrapperType, const RequestType&>)
				{
					return this->set_result(this->predicate(this->get_request()));
				}
				else if constexpr (std::is_invocable_v<Base::PredicateWrapperType>)
				{
					return this->set_result(this->predicate());
				}
				else
				{
					return this->set_result(false);
				}
			}

			bool operator()(const MetaAny& opaque_value) override
			{
				return operator()();
			}

			bool operator()(const Script& self, const MetaAny& opaque_value) override
			{
				assert((&self) == (&(this->get_script())));

				return operator()(opaque_value);
			}
	};
}