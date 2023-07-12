#pragma once

#include "standard_binary_stream.hpp"

#include <fstream>

namespace util
{
	using BinaryFileStream = StandardBinaryStreamWrapper<std::ifstream, std::ofstream>;

	using BinaryInputFileStream = BinaryInputStreamWrapper<std::ifstream>;
	using BinaryOutputFileStream = BinaryOutputStreamWrapper<std::ofstream>;
}