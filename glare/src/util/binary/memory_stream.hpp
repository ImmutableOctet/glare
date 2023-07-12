#pragma once

#include "types.hpp"

#include "fixed_binary_stream.hpp"

#include <utility>

namespace util
{
	class OwningMemoryStream : public FixedBinaryStream
	{
		public:
			inline OwningMemoryStream(OwningBinaryDataBuffer&& buffer, bool sync_input_and_output=true)
				: FixedBinaryStream(std::move(buffer), sync_input_and_output) {}

			inline explicit OwningMemoryStream(std::size_t buffer_size, bool sync_input_and_output=true)
				: OwningMemoryStream(OwningBinaryDataBuffer(buffer_size), sync_input_and_output) {}

			//inline OwningMemoryStream(bool sync_input_and_output=true)
			//	: OwningMemoryStream(1024, sync_input_and_output) {}

			OwningMemoryStream(OwningMemoryStream&&) noexcept = default;
			OwningMemoryStream& operator=(OwningMemoryStream&&) noexcept = default;

			//OwningMemoryStream(const OwningMemoryStream&) = delete;
			//const OwningMemoryStream& operator=(const OwningMemoryStream&) = delete;
	};

	using MemoryStream = OwningMemoryStream;
}

/*
namespace engine
{
	using Packet = MemoryStream;
}
*/