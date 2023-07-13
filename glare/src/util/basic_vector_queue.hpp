#pragma once

#include <queue>
#include <type_traits>

#include <cassert>

namespace util
{
	namespace impl
	{
		// A thin wrapper around a `std::vector`-like type allowing for use with `std::queue`.
		// 
		// NOTE: Use of inheritance for brevity.
		// (May require changes later if any conflicts arise with STL or third-party `vector` implementations, etc.)
		template
		<
			typename VectorType,

			bool assign_popped_to_default_constructed=(!std::is_arithmetic_v<typename VectorType::value_type>)
		>
		class vector_to_queue_interface : public VectorType
		{
			public:
				using size_type = std::size_t; // typename VectorType::size_type;

			private:
				using IndexType = size_type; // typename VectorType::size_type;

				IndexType queue_front_index = {};

			public:
				using VectorType::VectorType;

				constexpr VectorType::reference front()
				{
					assert(queue_front_index < static_cast<IndexType>(VectorType::size()));

					return (*this)[static_cast<size_type>(queue_front_index)];
				}

				constexpr VectorType::const_reference front() const
				{
					assert(queue_front_index < static_cast<IndexType>(VectorType::size()));

					return (*this)[static_cast<size_type>(queue_front_index)];
				}

				constexpr void pop_front()
				{
					if (VectorType::empty())
					{
						return;
					}

					const auto next_front_index = (queue_front_index + static_cast<IndexType>(1));

					if (next_front_index > static_cast<IndexType>(VectorType::size()))
					{
						return;
					}

					if constexpr (assign_popped_to_default_constructed)
					{
						(*this)[queue_front_index] = typename VectorType::value_type {};
					}

					queue_front_index = next_front_index;
				}

				constexpr bool empty() const
				{
					return (queue_front_index >= VectorType::size());
				}
		};
	}

	template <typename VectorType>
	using basic_vector_queue = std::queue<typename VectorType::value_type, impl::vector_to_queue_interface<VectorType>>;
}