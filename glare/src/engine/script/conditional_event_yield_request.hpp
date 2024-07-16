#pragma once

#include "script_interface.hpp"
#include "conditional_yield_request.hpp"
#include "event_yield_request.hpp"
#include "predicate.hpp"

#include <engine/meta/types.hpp>

#include <cassert>

namespace engine
{
	class Script;

	template <typename EventType, typename PredicateType>
	class ConditionalEventYieldRequest : public ConditionalYieldRequestImpl<EventYieldRequest, PredicateType, &impl::generate_continuation_predicate_wrapper<EventType, PredicateType>>
	{
		public:
			static_assert(!std::is_reference_v<EventType>, "`EventType` must be a non-reference type.");

			using ResumeType = const EventType&;

			using Base = ConditionalEventYieldRequest::ConditionalYieldRequestImpl;

			ConditionalEventYieldRequest(Script& script, EventYieldRequest yield_request, PredicateType predicate) :
				Base(script, std::move(yield_request), std::move(predicate))
			{}

			ConditionalEventYieldRequest(EventYieldRequest yield_request, PredicateType predicate) :
				Base(std::move(yield_request), std::move(predicate))
			{}

			const EventType* try_get_result() const
			{
				const auto& script = Base::get_script_interface();

				const auto event_ref = script.get_captured_event();

				return event_ref.try_cast<EventType>();
			}

			const EventType& get_result() const
			{
				auto captured_event = try_get_result();

				assert(captured_event);

				return *captured_event;
			}

			operator ResumeType() const
			{
				return get_result();
			}

			bool operator()() const
			{
				return Base::operator()();
			}

			bool operator()(const MetaAny& opaque_value)
			{
				if (!Base::get_boolean_result())
				{
					Base::set_result(this->predicate(this->get_script(), opaque_value));
				}

				return this->operator()();
			}

			bool operator()(const Script& self, const MetaAny& opaque_value)
			{
				assert((&self) == (&(this->get_script())));

				return this->operator()(opaque_value);
			}
	};
}