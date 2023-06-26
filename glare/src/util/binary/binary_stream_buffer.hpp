#pragma once

#include "types.hpp"

#include <utility>

namespace util
{
	class BinaryStreamBuffer
	{
		public:
			inline virtual ~BinaryStreamBuffer() {}

			virtual Byte* data() = 0;
			virtual std::size_t size() const = 0;

			inline virtual const Byte* data() const
			{
				return const_cast<const Byte*>(const_cast<BinaryStreamBuffer*>(this)->data());
			}

			inline virtual Byte* get(std::size_t position, std::size_t count)
			{
				return const_cast<Byte*>(const_cast<const BinaryStreamBuffer*>(this)->get(position, count));
			}

			inline virtual const Byte* get(std::size_t position, std::size_t count) const
			{
				if (const auto end_position = (position + count); end_position <= size())
				{
					return (data() + position);
				}

				return {};
			}

			inline bool empty() const
			{
				return (size() > 0);
			}

			inline explicit operator bool() const
			{
				return (!empty());
			}
	};
}