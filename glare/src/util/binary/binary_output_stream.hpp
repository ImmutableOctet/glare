#pragma once

#include "types.hpp"

#include <util/byte_order.hpp>

#include <utility>
#include <type_traits>
#include <string>
#include <string_view>
#include <stdexcept>

namespace util
{
	class BinaryStreamBuffer;

	class BinaryOutputStream
	{
		protected:
			using StringLengthType = std::uint32_t; // std::uint16_t;

			template <typename T, bool error_on_failure=true>
			BinaryOutputStream& basic_write_from_trivial_impl(const T& value)
			{
				constexpr auto bytes_to_copy = sizeof(std::decay_t<T>);

				bool result = false;

				if constexpr (std::is_arithmetic_v<T> && is_little_endian())
				{
					if (use_network_byte_order)
					{
						const auto value_byte_swapped = host_to_network_byte_order(value);

						result = write_bytes(reinterpret_cast<const Byte*>(&value_byte_swapped), bytes_to_copy);
					}
				}

				if (!result)
				{
					result = write_bytes(reinterpret_cast<const Byte*>(&value), bytes_to_copy);
				}

				if (!result)
				{
					if constexpr (error_on_failure)
					{
						throw std::runtime_error("Unable to complete write operation");
					}
				}

				return *this;
			}

			template <typename StringType, bool error_on_failure=true>
			BinaryOutputStream& basic_write_from_string_impl(const StringType& value)
			{
				const auto bytes_to_copy = value.size(); // length();
				const auto length = static_cast<StringLengthType>(bytes_to_copy);

				write(length);

				if (!write_bytes(reinterpret_cast<const Byte*>(value.data()), bytes_to_copy))
				{
					if constexpr (error_on_failure)
					{
						throw std::runtime_error("Unable to complete write operation");
					}
				}

				return *this;
			}

			template <typename T, bool error_on_failure=true>
			BinaryOutputStream& basic_write_from_impl(const T& value)
			{
				if constexpr (std::is_same_v<std::decay_t<T>, std::string> || std::is_same_v<std::decay_t<T>, std::string_view>)
				{
					return basic_write_from_string_impl<T, error_on_failure>(value);
				}
				else
				{
					return basic_write_from_trivial_impl<T, error_on_failure>(value);
				}
			}

			virtual bool write_bytes(const Byte* data_in, std::size_t count) = 0;

			bool use_network_byte_order : 1 = false;

		public:
			inline BinaryOutputStream(bool use_network_byte_order=false) :
				use_network_byte_order(use_network_byte_order)
			{}

			inline virtual ~BinaryOutputStream() {}

			// Optionally returns a pointer to the underlying output stream buffer.
			inline virtual BinaryStreamBuffer* get_output_buffer()
			{
				return {};
			}

			// Returns the position (offset) of the output stream relative to the origin.
			virtual StreamPosition get_output_position() const = 0;

			// Attempts to set the output stream position to `position`.
			// 
			// The return value indicates success/failure.
			inline virtual bool set_output_position(StreamPosition position)
			{
				return false;
			}

			inline virtual bool can_write(std::size_t count) const
			{
				return true;
			}

			inline virtual bool can_write() const
			{
				return can_write(static_cast<std::size_t>(1));
			}

			template <typename T>
			bool can_write_value() const
			{
				return can_write(sizeof(T));
			}

			// Generalized implementation of `write_from`.
			// (Calls `basic_read_to_impl` internally)
			// 
			// `T` must be a trivially copyable type.
			template
			<
				typename T,

				bool error_on_failure=true,

				typename=std::enable_if_t<std::is_trivially_copyable_v<T>>
			>
			BinaryOutputStream& write_from(const T& value)
			{
				return basic_write_from_impl<T, error_on_failure>(value);
			}

			inline virtual BinaryOutputStream& write_from(const std::string& value)
			{
				constexpr bool error_on_failure = true;

				return basic_write_from_impl<decltype(value), error_on_failure>(value);
			}

			inline virtual BinaryOutputStream& write_from(std::string_view value)
			{
				constexpr bool error_on_failure = true;

				return basic_write_from_impl<decltype(value), error_on_failure>(value);
			}

			// Alias to `write_from`.
			template <typename T>
			BinaryOutputStream& write(T&& value)
			{
				return write_from(std::forward<T>(value));
			}

			// Utility operator for `write_from`.
			template <typename T>
			BinaryOutputStream& operator<<(const T& value)
			{
				return write_from(value);
			}

			inline explicit operator bool() const
			{
				return can_write();
			}

			// This acts as shorthand for `set_output_position`.
			inline bool seek_output(StreamPosition position)
			{
				return set_output_position(position);
			}

			// This acts as shorthand for `get_output_position`.
			inline StreamPosition tellp() const
			{
				return get_output_position();
			}

			// This acts as shorthand for `set_output_position`.
			inline bool seekp(StreamPosition position)
			{
				return set_output_position(position);
			}
	};
}