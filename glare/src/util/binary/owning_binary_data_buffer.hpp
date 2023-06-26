#pragma once

#include "types.hpp"

#include "binary_stream_buffer.hpp"

#include <utility>
#include <memory>

//#include <algorithm>

#include <cstring>
#include <cassert>

namespace util
{
	class OwningBinaryDataBuffer : public BinaryStreamBuffer
	{
		protected:
			using RemoteData = std::unique_ptr<Byte[]>;
			
			inline static RemoteData& copy_data(const RemoteData& source, RemoteData& destination, std::size_t count, std::size_t source_offset=0, std::size_t destination_offset=0)
			{
				if ((source) && (destination) && (count))
				{
					std::memcpy((destination.get() + destination_offset), (source.get() + source_offset), count);
				}

				return destination;
			}

			inline static RemoteData allocate_copy(const RemoteData& source, std::size_t source_size)
			{
				assert(source);

				auto destination = std::make_unique<Byte[]>(source_size);

				assert(destination);

				copy_data(source, destination, source_size);

				return destination;
			}

			inline static RemoteData allocate_copy(const OwningBinaryDataBuffer& source)
			{
				return allocate_copy(source.remote_data, source.remote_data_size);
			}

			friend void swap(OwningBinaryDataBuffer& left, OwningBinaryDataBuffer& right);

		public:
			OwningBinaryDataBuffer() = default;

			inline explicit OwningBinaryDataBuffer(std::size_t remote_data_size) :
				remote_data(std::make_unique<Byte[]>(remote_data_size)),
				remote_data_size(remote_data_size)
			{}

			inline OwningBinaryDataBuffer(RemoteData&& remote_data, std::size_t remote_data_size) :
				remote_data(std::move(remote_data)), remote_data_size(remote_data_size)
			{}

			// Constructs a new buffer with a deep copy of `source`.
			inline OwningBinaryDataBuffer(const OwningBinaryDataBuffer& source) :
				remote_data(allocate_copy(source)), remote_data_size(source.size())
			{}

			inline OwningBinaryDataBuffer(OwningBinaryDataBuffer&& source) noexcept :
				OwningBinaryDataBuffer()
			{
				swap(*this, source);
			}

			// Performs a deep-copy of the contents of `source`, allocating a new buffer if necessary.
			inline OwningBinaryDataBuffer& operator=(const OwningBinaryDataBuffer& source)
			{
				if (this->remote_data_size == source.remote_data_size)
				{
					copy(source);
				}
				else
				{
					this->remote_data = allocate_copy(source.remote_data, source.remote_data_size);
					this->remote_data_size = source.remote_data_size;
				}

				return *this;
			}

			inline OwningBinaryDataBuffer& operator=(OwningBinaryDataBuffer&& source) noexcept
			{
				swap(*this, source);

				return *this;
			}

			inline OwningBinaryDataBuffer& copy
			(
				const OwningBinaryDataBuffer& source,

				std::size_t source_offset=0,
				std::size_t destination_offset=0
			)
			{
				if (!source)
				{
					return *this;
				}

				if (destination_offset >= this->remote_data_size)
				{
					return *this;
				}

				if (source_offset >= source.remote_data_size)
				{
					return *this;
				}

				const auto destination_count = (this->remote_data_size - destination_offset);
				const auto source_count = (source.remote_data_size - source_offset);

				const auto size_to_copy = (source_count < destination_count) // std::min(source_count, destination_count);
					? source_count
					: destination_count
				;

				copy_data(source.remote_data, this->remote_data, size_to_copy, source_offset, destination_offset);

				return *this;
			}

			inline Byte* data() override
			{
				return remote_data.get();
			}

			inline std::size_t size() const override
			{
				if (!has_data())
				{
					return 0;
				}

				return remote_data_size;
			}

		protected:
			inline bool has_data() const
			{
				return (static_cast<bool>(remote_data));
			}

			inline bool has_size() const
			{
				return (remote_data_size > 0);
			}

			RemoteData remote_data;

			std::size_t remote_data_size = 0;
	};

	void swap(OwningBinaryDataBuffer& left, OwningBinaryDataBuffer& right)
	{
		using std::swap;

		swap(left.remote_data, right.remote_data);
		swap(left.remote_data_size, right.remote_data_size);
	}
}