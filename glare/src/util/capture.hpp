#pragma once

#include <tuple>
#include <optional>
#include <utility>

#include <cassert>

namespace util
{
	template <typename... CaptureDataTypes>
	struct Capture
	{
		private:
			using CaptureDataTupleType = std::tuple<CaptureDataTypes...>;
			using CaptureDataStorageType = std::optional<CaptureDataTupleType>;

		protected:
			CaptureDataStorageType _capture;

		public:
			using CaptureDataTuple = CaptureDataTupleType;
			using CaptureDataRef = CaptureDataTupleType&;
			using OptionalCaptureDataRef = CaptureDataStorageType&; // CaptureDataTupleType*

			Capture() = default;

			Capture(CaptureDataTupleType&& capture_data)
				: _capture(std::move(capture_data))
			{}

			void capture(auto&&... capture_data)
			{
				_capture.emplace(std::make_tuple(std::forward<decltype(capture_data)>(capture_data)...));
			}

			bool try_apply_capture(auto&& callback)
			{
				if (auto&& capture_data = try_get_capture())
				{
					std::apply(std::forward<decltype(callback)>(callback), *capture_data);

					return true;
				}

				return false;
			}

			decltype(auto) apply_capture(auto&& callback)
			{
				return std::apply(std::forward<decltype(callback)>(callback), get_capture());
			}

			bool has_capture() const
			{
				return (_capture.has_value());
			}

			OptionalCaptureDataRef try_get_capture()
			{
				return _capture;
			}

			const OptionalCaptureDataRef try_get_capture() const
			{
				return _capture;
			}

			CaptureDataRef get_capture()
			{
				assert(has_capture());

				return *_capture;
			}

			const CaptureDataRef get_capture() const
			{
				assert(has_capture());

				return *_capture;
			}
	};
}