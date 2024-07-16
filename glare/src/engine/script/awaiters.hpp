#pragma once

#include "predicate.hpp"
#include "request_awaiter.hpp"
#include "request_awaiter_wrappable.hpp"
#include "immediate_yield_request.hpp"

namespace engine
{
	class Script;

	template <typename RequestType>
	using ConditionalRequestAwaiter = RequestAwaiter
	<
		RequestType,

		&impl::register_continuation_predicate_ref<Script, RequestType&>
	>;

	template <typename RequestType>
	using ImmediateRequestAwaiter = RequestAwaiter
	<
		ImmediateYieldRequest<RequestType>,

		nullptr
	>;
}