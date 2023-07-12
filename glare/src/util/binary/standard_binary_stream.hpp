#pragma once

#include "binary_stream.hpp"

#include "standard_binary_input_stream.hpp"
#include "standard_binary_output_stream.hpp"

#include <type_traits>

#include <istream>
#include <ostream>
#include <iostream>

namespace util
{

#ifdef _MSC_VER
	// Disable unhelpful diamond inheritance warning on MSVC.
	#pragma warning(push)
	#pragma warning(disable : 4250)
#endif

	template <typename InputStreamWrapper, typename OutputStreamWrapper>
	class StandardBinaryStreamImpl :
		public virtual BinaryStream,
		public virtual InputStreamWrapper,
		public virtual OutputStreamWrapper
	{
		public:
			using NativeInputStream = typename InputStreamWrapper::NativeInputStream;
			using NativeOutputStream = typename OutputStreamWrapper::NativeOutputStream;

			using GenericNativeIOStream = std::iostream;

			StandardBinaryStreamImpl(NativeInputStream& input_stream, NativeOutputStream& output_stream) : 
				InputStreamWrapper(input_stream),
				OutputStreamWrapper(output_stream)
			{}

			template
			<
				typename IOStreamType,

				typename=std::enable_if_t
				<
					(
						(std::is_same_v<std::decay_t<IOStreamType>, GenericNativeIOStream>)
						||
						(std::is_base_of_v<GenericNativeIOStream, std::decay_t<IOStreamType>>)
						||
						(
							(std::is_base_of_v<std::istream, std::decay_t<IOStreamType>>)
							&&
							(std::is_base_of_v<std::ostream, std::decay_t<IOStreamType>>)
						)
					)
				>
			>
			StandardBinaryStreamImpl(IOStreamType& input_output_stream) :
				InputStreamWrapper(input_output_stream),
				OutputStreamWrapper(input_output_stream)
			{}

			bool is_input_stream() const override
			{
				return true;
			}

			bool is_output_stream() const override
			{
				return true;
			}
	};

#ifdef _MSC_VER
	#pragma warning(pop)
#endif

	// A binary I/O interface that wraps a pair of standard streams. (uses `std::istream` and `std::ostream`)
	using StandardBinaryStream = StandardBinaryStreamImpl<StandardBinaryInputStream, StandardBinaryOutputStream>;

	// Creates a binary I/O interface that wraps a pair of standard stream types.
	template <typename StandardIStreamType, typename StandardOStreamType>
	using StandardBinaryStreamWrapper = StandardBinaryStreamImpl<BinaryInputStreamWrapper<StandardIStreamType>, BinaryOutputStreamWrapper<StandardOStreamType>>;
}