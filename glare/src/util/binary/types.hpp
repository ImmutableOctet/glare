#pragma once

#include <cstdint>
#include <cstddef>

namespace util
{
	class OwningBinaryDataBuffer;

	using BinaryDataBuffer = OwningBinaryDataBuffer;

	using Byte = std::uint8_t;
	using StreamPosition = std::size_t;
}