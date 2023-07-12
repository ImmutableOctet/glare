#pragma once

#include "binary_input_stream.hpp"

#include <utility>
#include <istream>

namespace util
{
	template <typename StandardIStreamType, typename SuperType=BinaryInputStream>
	class BinaryInputStreamWrapper :
		public virtual SuperType
	{
		public:
			using NativeInputStream = StandardIStreamType;

			template <typename ...Args>
			BinaryInputStreamWrapper(NativeInputStream& input_stream, Args&&... args) :
				SuperType(std::forward<Args>(args)...),
				input_stream(&input_stream)
			{}

			virtual ~BinaryInputStreamWrapper() {}

			bool end_of_file() const override
			{
				if (!input_stream)
				{
					return true;
				}

				return !static_cast<bool>(*input_stream);
			}

			StreamPosition get_input_position() const override
			{
				if (!input_stream)
				{
					return {};
				}

				return static_cast<StreamPosition>(input_stream->tellg());
			}

			bool set_input_position(StreamPosition position) const override
			{
				if (!input_stream)
				{
					return false;
				}

				return static_cast<bool>(input_stream->seekg(position));
			}

		protected:
			bool read_bytes(Byte* data_out, std::size_t count) const override
			{
				if (!input_stream)
				{
					return false;
				}

				return static_cast<bool>(input_stream->read(reinterpret_cast<char*>(data_out), count));
			}

			mutable NativeInputStream* input_stream = {};
	};

	using StandardBinaryInputStream = BinaryInputStreamWrapper<std::istream>;
}