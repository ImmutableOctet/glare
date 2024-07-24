#pragma once

#include "time_decl.hpp"

#include "types.hpp"

#include "meta/hash.hpp"

#include <optional>
#include <chrono>
#include <ratio>

namespace engine
{
	namespace time
	{
		namespace shared
		{
			namespace impl
			{
				template <typename OutputDurationType, typename FloatType, typename ToDurationRawFn>
				std::optional<OutputDurationType> to_duration_float_impl(FloatType duration_raw, StringHash time_symbol_id, ToDurationRawFn&& to_duration_from_raw)
				{
					using namespace engine::literals;

					auto convert = [duration_raw]<typename OutType, typename RatioType=typename OutType::period>()
					{
						constexpr auto duration_ratio = (static_cast<FloatType>(OutputDurationType::period::num) / static_cast<FloatType>(OutputDurationType::period::den));
						constexpr auto local_scale_ratio = (static_cast<FloatType>(RatioType::num) / static_cast<FloatType>(RatioType::den));
						constexpr auto time_symbol_inversion_ratio = (local_scale_ratio / duration_ratio);

						auto raw_out = static_cast<OutType::rep>(duration_raw * time_symbol_inversion_ratio);

						return OutputDurationType(raw_out);
					};

					switch (time_symbol_id)
					{
						case "d"_hs:
							return convert.template operator()<Days>();
						case "h"_hs:
							return convert.template operator()<Hours>();
						case "m"_hs:
							return convert.template operator()<Minutes>();
						case "s"_hs:
							return to_duration_from_raw(duration_raw); // return convert.template operator()<Seconds>();
						case "ms"_hs:
							return convert.template operator()<Milliseconds>(); // std::milli
						case "us"_hs:
							return convert.template operator()<Microseconds>();
					}

					return std::nullopt;
				}

				template <typename InputDurationTypeRaw, typename ToDurationFn>
				decltype(auto) to_duration_dispatch_impl(InputDurationTypeRaw duration_raw, std::string_view time_symbol, ToDurationFn&& to_duration)
				{
					using namespace engine::literals;

					const auto time_symbol_id = (time_symbol.empty())
						? "s"_hs
						: hash(time_symbol)
					;

					return to_duration(duration_raw, time_symbol_id);
				}

				template <typename OutputDurationType, typename DurationTypeRaw>
				std::optional<OutputDurationType> to_duration_impl(DurationTypeRaw duration_rep, StringHash time_symbol_id)
				{
					using namespace engine::literals;

					switch (time_symbol_id)
					{
						case "d"_hs:
							return Days(duration_rep);
						case "h"_hs:
							return Hours(duration_rep);
						case "m"_hs:
							return Minutes(duration_rep);
						case "s"_hs:
							return Seconds(duration_rep);
						case "ms"_hs:
							return Milliseconds(duration_rep);
						case "us"_hs:
							return Microseconds(duration_rep);
					}

					return std::nullopt;
				}
			}
		}
	}
}