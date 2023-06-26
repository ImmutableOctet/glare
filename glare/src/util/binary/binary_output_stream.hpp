#pragma once

#include "types.hpp"

#include <util/byte_order.hpp>

#include <utility>
#include <type_traits>
#include <stdexcept>

namespace util
{
	class BinaryStreamBuffer;

	class BinaryOutputStream
	{
		protected:
			template <typename T, bool error_on_failure=true>
			BinaryOutputStream& basic_write_from_impl(const T& value)
			{
				constexpr auto bytes_to_copy = sizeof(std::decay_t<T>);

				bool result = false;

				if constexpr (std::is_arithmetic_v<T> && is_little_endian())
				{
					const auto value_byte_swapped = host_to_network_byte_order(value);

					result = write_bytes(reinterpret_cast<const Byte*>(&value_byte_swapped), bytes_to_copy);
				}
				else
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

			// This acts as shorthand for `set_output_position`.
			inline bool seek_output(StreamPosition position)
			{
				return set_output_position(position);
			}
	};
}