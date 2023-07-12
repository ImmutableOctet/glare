#pragma once

#include "binary_stream.hpp"

#include "owning_binary_data_buffer.hpp"

#include <utility>
#include <type_traits>

#include <cstdint>
#include <cstring>

namespace util
{
	// Implements simple I/O operations for a fixed-size memory buffer.
	// 
	// TODO: Look into having the `sync_input_and_output` flag control
	// whether network byte-order remains in sync.
	template <typename BufferType, typename SuperStreamType=BinaryStream>
	class FixedBinaryStreamImpl : public SuperStreamType
	{
		private:
			// Internal const-qualified routine to update the mutable `output_position` field.
			bool set_output_position_impl(StreamPosition position) const
			{
				output_position = position;

				return true;
			}

			// Internal const-qualified routine to update the mutable `input_position` field.
			bool set_input_position_impl(StreamPosition position) const
			{
				input_position = position;

				return true;
			}

			mutable StreamPosition input_position = {}; // std::atomic<StreamPosition>
			mutable StreamPosition output_position = {}; // std::atomic<StreamPosition>

		protected:
			BufferType buffer;

			bool sync_input_and_output : 1 = true;

			const Byte* read_byte_segment(std::size_t count) const
			{
				if (const auto byte_segment = buffer.get(input_position, count))
				{
					if (set_input_position(input_position + static_cast<StreamPosition>(count)))
					{
						return byte_segment;
					}
				}

				return {};
			}

			bool read_bytes(Byte* data_out, std::size_t count) const override
			{
				if (const auto bytes_in = read_byte_segment(count))
				{
					std::memcpy(data_out, bytes_in, count);

					return true;
				}

				return false;
			}

			bool write_bytes(const Byte* data_in, std::size_t count) override
			{
				if (auto bytes_out = buffer.get(output_position, count))
				{
					if (set_output_position(output_position + static_cast<StreamPosition>(count)))
					{
						std::memcpy(bytes_out, data_in, count);

						return true;
					}
				}

				return false;
			}

			// Optimal buffer-to-string copy routine. (Avoids temporary buffer creation)
			const BinaryInputStream& read_to(std::string& out) const override
			{
				auto as_input_stream = static_cast<const BinaryInputStream*>(this);

				const auto length = as_input_stream->read<std::uint32_t>(); // std::uint16_t

				if (length)
				{
					const auto buffer_size = static_cast<std::size_t>(length);
					const auto buffer_segment = read_byte_segment(buffer_size);

					//out.clear();
					out.resize(buffer_size);

					std::copy(buffer_segment, (buffer_segment + buffer_size), out.begin());
				}
				else
				{
					out.clear();
				}

				return *this;
			}

		public:
			template <typename ...SharedArgs>
			FixedBinaryStreamImpl(BufferType&& buffer, bool sync_input_and_output, SharedArgs&&... shared_args) :
				SuperStreamType(std::forward<SharedArgs>(shared_args)...),

				buffer(std::move(buffer)),
				sync_input_and_output(sync_input_and_output)
			{}

			FixedBinaryStreamImpl(BufferType&& buffer) :
				FixedBinaryStreamImpl(std::move(buffer), true) {}

			template <typename SharedArgs..., typename=std::enable_if_t<std::is_default_constructible_v<BufferType>>>
			FixedBinaryStreamImpl(bool sync_input_and_output, SharedArgs&&... shared_args) :
				SuperStreamType(std::forward<SharedArgs>(shared_args)...),

				sync_input_and_output(sync_input_and_output)
			{}

			template <typename=std::enable_if_t<std::is_default_constructible_v<BufferType>>>
			FixedBinaryStreamImpl() :
				FixedBinaryStreamImpl(true) {}

			FixedBinaryStreamImpl(const FixedBinaryStreamImpl&) = delete;
			FixedBinaryStreamImpl(FixedBinaryStreamImpl&&) noexcept = default;

			virtual ~FixedBinaryStreamImpl() {}

			FixedBinaryStreamImpl& operator=(const FixedBinaryStreamImpl&) = delete;
			FixedBinaryStreamImpl& operator=(FixedBinaryStreamImpl&&) noexcept = default;

			BufferType& get_native_buffer()
			{
				return buffer;
			}

			const BufferType& get_native_buffer() const
			{
				return buffer;
			}

			BinaryStreamBuffer* get_input_buffer() override
			{
				return &(get_native_buffer());
			}

			BinaryStreamBuffer* get_output_buffer() override
			{
				return &(get_native_buffer());
			}

			bool set_input_position(StreamPosition position) const override
			{
				set_input_position_impl(position);

				if (sync_input_and_output)
				{
					return set_output_position_impl(position);
				}

				return true;
			}

			StreamPosition get_input_position() const override
			{
				return input_position;
			}

			bool set_output_position(StreamPosition position) override
			{
				set_output_position_impl(position);

				if (sync_input_and_output)
				{
					return set_input_position_impl(position);
				}

				return true;
			}

			StreamPosition get_output_position() const override
			{
				return output_position;
			}

			bool end_of_file() const override
			{
				return (static_cast<std::size_t>(input_position) >= size());
			}

			bool is_input_stream() const override
			{
				return true;
			}

			bool is_output_stream() const override
			{
				return true;
			}

			inline Byte* data()
			{
				return buffer.data();
			}

			inline const Byte* data() const
			{
				return buffer.data();
			}

			inline std::size_t size() const
			{
				return buffer.size();
			}
	};

	using FixedOwningBinaryStream = FixedBinaryStreamImpl<OwningBinaryDataBuffer>;

	using FixedBinaryStream = FixedOwningBinaryStream;
}