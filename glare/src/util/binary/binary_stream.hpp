#pragma once

#include "types.hpp"

#include "binary_input_stream.hpp"
#include "binary_output_stream.hpp"

#include <utility>
#include <stdexcept>

namespace util
{
	class BinaryStreamBuffer;

	class BinaryStream :
		public virtual BinaryInputStream,
		public virtual BinaryOutputStream
	{
		public:
			inline BinaryStream(bool use_network_byte_order=false) :
				BinaryInputStream(use_network_byte_order),
				BinaryOutputStream(use_network_byte_order)
			{}

			inline virtual ~BinaryStream() {}
			
			virtual bool is_input_stream() const = 0;
			virtual bool is_output_stream() const = 0;

			/*
			BinaryStream(const BinaryStream&) = delete;
			BinaryStream(BinaryStream&&) noexcept = default;

			BinaryStream& operator=(const BinaryStream&) = delete;
			BinaryStream& operator=(BinaryStream&&) noexcept = default;
			*/

			// Attempts to set both the input and output positions for this stream.
			inline virtual bool seek(StreamPosition position)
			{
				return
				(
					((!is_input_stream()) || (set_input_position(position)))
					||
					((!is_output_stream()) || (set_output_position(position)))
				);
			}

			// Gets the input stream's network byte-order flag.
			inline bool get_input_network_byte_order() const
			{
				return BinaryInputStream::get_network_byte_order();
			}

			// Gets the output stream's network byte-order flag.
			inline bool get_output_network_byte_order() const
			{
				return BinaryOutputStream::get_network_byte_order();
			}

			// Sets the input stream's network byte-order flag.
			inline void set_input_network_byte_order(bool value)
			{
				BinaryInputStream::set_network_byte_order(value);
			}

			// Sets the output stream's network byte-order flag.
			inline void set_output_network_byte_order(bool value)
			{
				BinaryOutputStream::set_network_byte_order(value);
			}

			// Sets both the input and output streams' network byte-order flags to `value`.
			inline void set_network_byte_order(bool value)
			{
				set_input_network_byte_order(value);
				set_output_network_byte_order(value);
			}

			// Returns true if both the input and output streams are using network byte-order.
			inline bool get_network_byte_order() const
			{
				return (get_input_network_byte_order() && get_output_network_byte_order());
			}
	};
}