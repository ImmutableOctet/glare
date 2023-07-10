#pragma once

#include "types.hpp"

#include <util/byte_order.hpp>
#include <util/small_vector.hpp>

#include <type_traits>
#include <string>
#include <string_view>
#include <stdexcept>
#include <algorithm>

#include <cstdint>
#include <cstring>

namespace util
{
	class BinaryStreamBuffer;

	// NOTE: The 'read' API exposed by this type is considered `const`,
	// since it does not alter the underlying data source.
	// 
	// This does not mean the stream, as a viewer of this data source,
	// is not modified during these operations.
	class BinaryInputStream
	{
		protected:
			using StringLengthType = std::uint32_t; // std::uint16_t;

			using TemporaryBuffer = util::small_vector<Byte, 128>; // 32 // 64 // 1024

			template <typename T>
			const T& basic_read_impl() const
			{
				if (auto bytes = read_bytes(sizeof(T)))
				{
					return reinterpret_cast<const T&>(*bytes);
				}

				// NOTE: Placeholder error message.
				throw std::runtime_error("Failed to read from stream.");
			}

			template <typename T, bool error_on_failure=true>
			const BinaryInputStream& basic_read_to_trivial_impl(T& out) const
			{
				//out = basic_read_impl<std::decay_t<T>>(); // std::remove_reference_t<T>

				constexpr auto bytes_to_copy = sizeof(std::decay_t<T>);

				if (read_bytes(reinterpret_cast<Byte*>(&out), bytes_to_copy))
				{
					if constexpr (std::is_arithmetic_v<T> && is_little_endian())
					{
						if (use_network_byte_order)
						{
							out = network_to_host_byte_order(out);
						}
					}
				}
				else
				{
					if constexpr (error_on_failure)
					{
						throw std::runtime_error("Unable to complete read operation");
					}
					else if constexpr (std::is_default_constructible_v<T> && std::is_copy_assignable_v<T>)
					{
						out = T {};
					}
				}

				return *this;
			}

			template <typename StringType, bool error_on_failure = true>
			const BinaryInputStream& basic_read_to_string_impl(StringType& out) const
			{
				const auto length = read<StringLengthType>();

				if (length)
				{
					const auto temp_buffer = read_bytes(length);

					//out.clear();
					out.resize(static_cast<std::size_t>(length));
					
					std::copy(temp_buffer.begin(), temp_buffer.end(), out.begin());
				}
				else
				{
					out.clear();
				}

				return *this;
			}

			template <typename T, bool error_on_failure=true>
			const BinaryInputStream& basic_read_to_impl(T& out) const
			{
				// NOTE: `std::string_view` is not supported here since the `out`
				// object must own the data we populate it with.
				if constexpr (std::is_same_v<std::decay_t<T>, std::string>)
				{
					return basic_read_to_string_impl<T, error_on_failure>(out);
				}
				else
				{
					return basic_read_to_trivial_impl<T, error_on_failure>(out);
				}
			}

			virtual bool read_bytes(Byte* data_out, std::size_t count) const = 0;

			inline TemporaryBuffer read_bytes(std::size_t count) const
			{
				auto temporary_data = TemporaryBuffer {};

				read_bytes(temporary_data.data(), count);

				return temporary_data;
			}

			bool use_network_byte_order : 1 = false;

		public:
			inline BinaryInputStream(bool use_network_byte_order=false) :
				use_network_byte_order(use_network_byte_order)
			{}

			inline virtual ~BinaryInputStream() {}

			// Generalized implementation of `read_to`.
			// (Calls `basic_read_to_impl` internally)
			// 
			// `T` must be a trivially copyable type.
			template
			<
				typename T,

				bool error_on_failure=true,

				typename=std::enable_if_t<std::is_trivially_copyable_v<T>>
			>
			const BinaryInputStream& read_to(T& out) const
			{
				return basic_read_to_impl<T, error_on_failure>(out);
			}

			inline virtual const BinaryInputStream& read_to(std::string& out) const
			{
				constexpr bool error_on_failure = true;

				return basic_read_to_impl<decltype(out), error_on_failure>(out);
			}

			// Attempts to read a new instance of `T` from this stream.
			// 
			// `T` must be a trivially copyable type.
			// 
			// See also: `read_to`
			template
			<
				typename T,

				typename OutputType = std::conditional_t
				<
					// Allows for use of `std::string_view` as `T`, while providing `std::string` as output:
					(std::is_same_v<std::decay_t<T>, std::string_view>),
						std::string,
						
						T
				>
			>
			OutputType read() const
			{
				if constexpr (std::is_same_v<std::decay_t<OutputType>, std::string>)
				{
					auto instance = OutputType {};

					read_to(instance);

					return instance;
				}
				else if constexpr ((std::is_trivially_copyable_v<T>) && (std::is_default_constructible_v<T> || std::is_arithmetic_v<T>))
				{
					// On paper, this is slightly more efficient for primitives.
					// (Probably doesn't matter when optimized)
					//T instance;

					auto instance = T {};

					read_to(instance);

					if constexpr (std::is_same_v<std::decay_t<T>, std::decay_t<OutputType>>)
					{
						return instance;
					}
					else
					{
						return static_cast<OutputType>(instance);
					}
				}
				
				// TODO: Add checks for relevant member-functions.
				
				/*
				else if constexpr (std::is_copy_constructible_v<T>)
				{
					return basic_read_impl<std::decay_t<T>>();
				}
				else if constexpr (std::is_default_constructible_v<T> && std::is_copy_assignable_v<T>)
				{
					auto instance = T {};

					instance = basic_read_impl<std::decay_t<T>>();

					return instance;
				}

				else if constexpr (std::is_default_constructible_v<T>)
				{
					return T {};
				}
				*/

				else
				{
					static_assert(std::integral_constant<T, false>::value, "Unsupported type; `T` must be trivially copyable and default constructible.");
				}
			}


			// Utility operator for `read_to`.
			template <typename T>
			const BinaryInputStream& operator>>(T& out) const
			{
				return read_to(out);
			}

			// Optionally returns a pointer to the underlying input stream buffer.
			inline virtual BinaryStreamBuffer* get_input_buffer()
			{
				return {};
			}

			// Optionally returns a pointer to the underlying input stream buffer.
			inline virtual const BinaryStreamBuffer* get_input_buffer() const
			{
				return const_cast<const BinaryStreamBuffer*>(const_cast<BinaryInputStream*>(this)->get_input_buffer());
			}

			// Returns the position (offset) of the input stream relative to the origin.
			virtual StreamPosition get_input_position() const = 0;

			// Returns true if the end of the readable area has been reached.
			virtual bool end_of_file() const = 0;

			// Returns true if the stream can be read from.
			inline virtual bool can_read() const
			{
				return (!end_of_file());
			}

			// Attempts to set the input stream position to `position`.
			// 
			// The return value indicates success/failure.
			inline virtual bool set_input_position(StreamPosition position) const
			{
				return false;
			}

			// This acts as shorthand for `get_input_position`.
			inline StreamPosition tellg() const
			{
				return get_input_position();
			}

			// This acts as shorthand for `set_input_position`.
			inline bool seekg(StreamPosition position) const
			{
				return set_input_position(position);
			}

			// This acts as shorthand for `set_input_position`.
			inline bool seek_input(StreamPosition position) const
			{
				return set_input_position(position);
			}

			// This acts as shorthand for `end_of_file`.
			inline bool eof() const
			{
				return end_of_file();
			}

			inline explicit operator bool() const
			{
				return can_read();
			}
	};
}