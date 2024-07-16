#pragma once

#include "yield_request.hpp"

#include <utility>

namespace engine
{
	// A yield request object that always returns true,
	// providing the underlying `RequestType` when prompted.
	template <typename RequestType>
	class ImmediateYieldRequest : public YieldRequestImpl<RequestType>
	{
		public:
			using Base = ImmediateYieldRequest::YieldRequestImpl;

			ImmediateYieldRequest(RequestType yield_request) :
				Base(std::move(yield_request))
			{}

			ImmediateYieldRequest(Script& script, RequestType yield_request) :
				ImmediateYieldRequest(std::move(yield_request))
			{}

			using Base::operator bool;
			using Base::operator RequestType;

			using Base::operator();

			bool operator()() override
			{
				const bool old_result = get_boolean_result();

				set_result(true);

				return old_result;
			}

			bool get_boolean_result() const override
			{
				return result;
			}
		protected:
			RequestType decay() override
			{
				set_result(true);

				return Base::decay();
			}

			bool set_result(bool value)
			{
				result = value;

				return value;
			}

			bool result = false;
	};
}