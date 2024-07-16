#pragma once

#include "request_awaiter_wrappable.hpp"
#include "yield_request_interface.hpp"

namespace engine
{
	template <typename YieldRequestType>
	class YieldRequestImpl : public RequestAwaiterWrappable, public YieldRequestInterface
	{
		public:
			using RequestType = YieldRequestType;
			using ResumeType = void;

			YieldRequestImpl(RequestType yield_request) :
				yield_request(std::move(yield_request))
			{}

			YieldRequestImpl(YieldRequestImpl&&) noexcept = default;
			YieldRequestImpl(const YieldRequestImpl&) = delete;

			YieldRequestImpl& operator=(YieldRequestImpl&&) noexcept = default;
			YieldRequestImpl& operator=(const YieldRequestImpl&) = delete;

			const RequestType& get_request() const
			{
				return yield_request;
			}

			decltype(auto) get_response() const
			{
				if constexpr (std::is_same_v<std::remove_cvref_t<RequestType>, ScriptFiberResponse>)
				{
					return get_request();
				}
				else
				{
					static_assert((std::is_convertible_v<const RequestType&, ScriptFiberResponse>));

					return static_cast<ScriptFiberResponse>(get_request());
				}
			}

			using YieldRequestInterface::operator();
			
			/*
			// Disabled for now; overload resolution issues with decay-based version.
			operator const RequestType&() const&
			{
				return get_request();
			}
			*/

			template
			<
				typename T,

				typename std::enable_if
				<
					(
						(std::is_same_v<std::remove_cvref_t<T>, std::remove_cvref_t<RequestType>>)
						&&
						(
							(!std::is_same_v<std::remove_cvref_t<RequestType>, ScriptFiberResponse>)
						)
					),
					
					int
				>::type=0
			>
			operator T() &&
			{
				return decay();
			}

			template
			<
				typename T,
				
				typename std::enable_if
				<
					(std::is_same_v<std::remove_cvref_t<T>, ScriptFiberResponse>),
					
					int
				>::type=0
			>
			operator T() const&
			{
				return get_response();
			}

			template
			<
				typename T,

				typename std::enable_if
				<
					(
						(std::is_same_v<std::remove_cvref_t<T>, ScriptFiberResponse>)
						&&
						(
							(std::is_same_v<std::remove_cvref_t<RequestType>, ScriptFiberResponse>)
							||
							(std::is_convertible_v<std::remove_cvref_t<RequestType>, ScriptFiberResponse>)
						)
					),
					
					int
				>::type=0
			>
			operator T() &&
			{
				return decay_as_response();
			}
		protected:
			virtual RequestType decay()
			{
				return get_request();
			}

			virtual ScriptFiberResponse decay_as_response()
			{
				if constexpr (std::is_same_v<std::remove_cvref_t<RequestType>, ScriptFiberResponse>)
				{
					return decay();
				}
				else if constexpr (std::is_convertible_v<std::remove_cvref_t<RequestType>, ScriptFiberResponse>)
				{
					return static_cast<ScriptFiberResponse>(decay());
				}
				else
				{
					return ScriptFiberResponse { std::monostate {} };
				}
			}

			RequestType yield_request;
	};
}