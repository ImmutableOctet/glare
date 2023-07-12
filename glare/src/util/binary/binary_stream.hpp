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
	};
}