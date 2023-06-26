#pragma once

#include "binary_output_stream.hpp"

#include <utility>
#include <ostream>

namespace util
{
	template <typename StandardOStreamType, typename SuperType=BinaryOutputStream>
	class BinaryOutputStreamWrapper :
		public virtual SuperType
	{
		public:
			using NativeOutputStream = StandardOStreamType;

			template <typename ...Args>
			BinaryOutputStreamWrapper(NativeOutputStream& output_stream, Args&&... args) :
				SuperType(std::forward<Args>(args)...),
				output_stream(&output_stream)
			{}

			virtual ~BinaryOutputStreamWrapper() {}

			StreamPosition get_output_position() const override
			{
				if (!output_stream)
				{
					return {};
				}

				return static_cast<StreamPosition>(output_stream->tellp());
			}

			bool set_output_position(StreamPosition position) override
			{
				if (!output_stream)
				{
					return false;
				}

				return static_cast<bool>(output_stream->seekp(position));
			}

		protected:
			bool write_bytes(const Byte* data_in, std::size_t count) override
			{
				if (!output_stream)
				{
					return false;
				}

				return static_cast<bool>(output_stream->write(reinterpret_cast<const char*>(data_in), count));
			}

			NativeOutputStream* output_stream = {};
	};

	using StandardBinaryOutputStream = BinaryOutputStreamWrapper<std::ostream>;
}