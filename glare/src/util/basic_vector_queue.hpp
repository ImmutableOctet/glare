#pragma once

#include <queue>
#include <type_traits>
#include <utility>

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
				using value_type = typename VectorType::value_type;
				using size_type = typename VectorType::size_type; // std::size_t;
				using reference = typename VectorType::reference;
				using const_reference = typename VectorType::const_reference;

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
					if (empty())
					{
						return;
					}

					const auto next_front_index = (queue_front_index + static_cast<IndexType>(1));

					/*
					if (next_front_index > static_cast<IndexType>(VectorType::size()))
					{
						return;
					}
					*/

					if constexpr (assign_popped_to_default_constructed)
					{
						(*this)[queue_front_index] = value_type {};
					}

					if (next_front_index >= static_cast<IndexType>(VectorType::size()))
					{
						clear();
					}
					else
					{
						queue_front_index = next_front_index;
					}
				}

				constexpr void push_back
				(
					std::enable_if_t
					<
						std::is_copy_constructible_v<value_type>,
						const value_type& // typename VectorType::const_reference
					>
					value
				)
				{
					if (empty())
					{
						clear();
					}

					VectorType::push_back(value);
				}

				constexpr void push_back
				(
					std::enable_if_t
					<
						std::is_move_constructible_v<value_type>,
						value_type&&
					>
					value
				)
				{
					if (empty())
					{
						clear();
					}

					VectorType::push_back(std::move(value));
				}

				template <typename ...Args>
				constexpr reference emplace_back(Args&&... args)
				{
					if (empty())
					{
						clear();
					}

					return VectorType::emplace_back(std::forward<Args>(args)...);
				}

				constexpr void clear()
				{
					VectorType::clear();

					queue_front_index = {};
				}

				constexpr size_type size() const
				{
					assert(static_cast<size_type>(queue_front_index) <= VectorType::size());

					return (VectorType::size() - static_cast<size_type>(queue_front_index));
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