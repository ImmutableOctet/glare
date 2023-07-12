#pragma once

#include "types.hpp"

#include "fixed_binary_stream.hpp"

#include <utility>

namespace util
{
	class OwningMemoryStream : public FixedBinaryStream
	{
		public:
			inline OwningMemoryStream(OwningBinaryDataBuffer&& buffer, bool sync_input_and_output=true, bool use_network_byte_order=false) :
				FixedBinaryStream(std::move(buffer), sync_input_and_output, use_network_byte_order) {}

			inline explicit OwningMemoryStream(std::size_t buffer_size, bool sync_input_and_output=true, bool use_network_byte_order=false) :
				OwningMemoryStream(OwningBinaryDataBuffer(buffer_size), sync_input_and_output, use_network_byte_order) {}

			//inline OwningMemoryStream(bool sync_input_and_output=true, bool use_network_byte_order=false)
			//	: OwningMemoryStream(1024, sync_input_and_output, use_network_byte_order) {}

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