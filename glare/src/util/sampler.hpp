#pragma once

#include "sampler_lerp_impl.hpp"
#include "type_traits.hpp"
#include "small_vector.hpp"

#include <math/types.hpp>
#include <math/math.hpp>

#include <utility>
#include <type_traits>
#include <algorithm>
#include <functional>
#include <span>

#include <cassert>
//#include <cmath>

namespace util
{
	namespace impl
	{
		/*
		template <typename KeyType, typename ValueType>
		struct Sampler_BasicEntry
		{
			KeyType key;
			ValueType value;
		};
		*/

		//template <typename KeyType, typename ValueType>
		//using Sampler_BasicEntry = std::pair<KeyType, ValueType>;

		// NOTE: Inheritance is used here to ensure a unique type signature.
		template <typename KeyType, typename ValueType> // typename LessImpl=std::less<KeyType>
		struct Sampler_BasicEntry : std::pair<KeyType, ValueType>
		{
			using pair_type = std::pair<KeyType, ValueType>;

			using pair_type::pair_type;

			Sampler_BasicEntry(const Sampler_BasicEntry&) = default;
			Sampler_BasicEntry(Sampler_BasicEntry&&) noexcept = default;

			constexpr Sampler_BasicEntry(pair_type&& content) noexcept
				: pair_type(std::move(content))
			{}

			constexpr Sampler_BasicEntry(const pair_type& content)
				: pair_type(content)
			{}

			using pair_type::operator=;
			//using pair_type::operator==;

			Sampler_BasicEntry& operator=(const Sampler_BasicEntry&) = default;
			Sampler_BasicEntry& operator=(Sampler_BasicEntry&&) noexcept = default;

			//auto operator<=>(const Sampler_BasicEntry&) const = default;

			bool operator==(const Sampler_BasicEntry&) const = default;
			bool operator!=(const Sampler_BasicEntry&) const = default;

			bool operator==(const pair_type& other) const
			{
				return (static_cast<const pair_type&>(*this) == other);
			}

			bool operator!=(const pair_type& other) const
			{
				return (static_cast<const pair_type&>(*this) != other);
			}
		};
	}

	template
	<
		typename KeyType, typename ValueType,

		typename ContainerType = small_vector<impl::Sampler_BasicEntry<KeyType, ValueType>, 8>, // 16 // Entry

		typename LessImpl = std::less<KeyType>,

		typename EntryType = typename ContainerType::value_type, // impl::Sampler_BasicEntry<KeyType, ValueType>,
		
		typename LerpImpl = std::conditional_t
		<
			std::is_same_v<std::decay_t<ValueType>, math::Quaternion>,
			
			impl::Sampler_Slerp<>,
			impl::Sampler_BasicLerp<>
		>,

		typename IndexType = std::size_t,
		typename FloatType = float
	>
	class SamplerImpl
	{
		public:
			using Container       = ContainerType;
			using Entry           = EntryType; // typename Container::value_type;
			using Sample          = Entry;

			using ConstIterator   = typename Container::const_iterator;

			// STL compatibility aliases:
			using size_type       = IndexType; // std::size_t;
			using iterator        = ConstIterator;
			using const_iterator  = ConstIterator;
			using key_type        = typename Entry::first_type;
			using value_type      = typename Entry::second_type;
			using const_reference = const Entry&;
			
			// Constructs an empty sampler object.
			SamplerImpl() = default;

			SamplerImpl(const SamplerImpl&) = default;
			SamplerImpl(SamplerImpl&&) noexcept = default;

			// Takes ownership of `data`, sorting if needed.
			SamplerImpl(Container&& data, bool is_sorted=false)
				: data(std::move(data))
			{
				if (!is_sorted)
				{
					sort();
				}
			}

			// Copies from `data`, sorting if needed.
			SamplerImpl(const Container& data, bool is_sorted=false)
				: data(data)
			{
				if (!is_sorted)
				{
					sort();
				}
			}

			// Copies from a span of size `span_size`, sorting if needed.
			template
			<
				typename SpanValueType,

				std::size_t span_size=std::dynamic_extent,

				typename = std::enable_if_t
				<
					(
						(std::is_constructible_v<Entry, const SpanValueType&>)
						||
						(std::is_assignable_v<Entry, const SpanValueType&>)
					)
				>
			>
			SamplerImpl(const std::span<SpanValueType, span_size>& data, bool is_sorted=false)
				: data(std::cbegin(data), std::cend(data))
			{
				if (!is_sorted)
				{
					sort();
				}
			}

			/*
			// Disabled for now:

			// Copies from an iterable input container (`data`)
			// that does not have a const-iterator available, sorting if needed.
			// 
			// NOTE: This overload handles an edge-case, and is rarely ADL-preferred.
			template
			<
				typename InputContainer,
				typename = std::enable_if_t
				<
					(
						(is_iterable_v<InputContainer>)
						&&
						(!is_const_iterable_v<InputContainer>)
						&&
						(
							(std::is_constructible_v<Entry, const typename InputContainer::value_type&>)
							||
							(std::is_assignable_v<Entry, const typename InputContainer::value_type&>)
						)
					)
				>
			>
			SamplerImpl(const InputContainer& data, bool is_sorted=false)
				: data(std::begin(data), std::end(data))
			{
				if (!is_sorted)
				{
					sort();
				}
			}
			*/

			// Copies from a const-iterable container (`data`), sorting if needed.
			template
			<
				typename InputContainer,
				typename = std::enable_if_t
				<
					(
						(is_const_iterable_v<InputContainer>)
						&&
						(
							(std::is_constructible_v<Entry, const typename InputContainer::value_type&>)
							||
							(std::is_assignable_v<Entry, const typename InputContainer::value_type&>)
						)
					)
				>
			>
			SamplerImpl(const InputContainer& data, bool is_sorted=false)
				: data(std::cbegin(data), std::cend(data))
			{
				if (!is_sorted)
				{
					sort();
				}
			}

			// Moves from the specified container (`data`) into an internal container.
			// The `MovedFromContainer` container must be iterable and the 
			template
			<
				typename MovedFromContainer,
				typename = std::enable_if_t
				<
					(
						(is_iterable_v<MovedFromContainer>)
						&&
						(
							(std::is_constructible_v<Entry, typename MovedFromContainer::value_type&&>)
							||
							(std::is_constructible_v<Entry, const typename MovedFromContainer::value_type&>)
							||
							(
								(std::is_assignable_v<Entry, typename MovedFromContainer::value_type&&>)
								||
								(std::is_assignable_v<Entry, const typename MovedFromContainer::value_type&>)
							)
						)
					)
				>
			>
			SamplerImpl(MovedFromContainer&& data, bool is_sorted=false)
			{
				std::move(std::begin(data), std::end(data), std::back_inserter(this->data));

				if (!is_sorted)
				{
					sort();
				}
			}

			// Copies data from the range between `begin_it` and `end_it`, sorting is needed.
			// 
			// `IteratorType` must be a valid iterator type.
			template
			<
				typename IteratorType,
				typename = std::enable_if_t<is_iterator_v<IteratorType>>
			>
			SamplerImpl(IteratorType&& begin_it, IteratorType&& end_it, bool is_sorted=false)
				: data(begin_it, end_it)
			{
				if (!is_sorted)
				{
					sort();
				}
			}

			SamplerImpl& operator=(const SamplerImpl&) = default;
			SamplerImpl& operator=(SamplerImpl&&) noexcept = default;

			// Retrieves a sample entry from the internal container using `index`.
			// NOTE: Samples are always sorted in ascending order along the 'X' axis.
			const Entry& get_entry_by_index(IndexType index) const
			{
				assert(index < data.size());

				return data[index];
			}

			// Retrieves a sample key from the internal container using `index`.
			// NOTE: Samples are always sorted in ascending order along the 'X' axis.
			const KeyType& get_key_by_index(IndexType index) const
			{
				return std::get<0>(get_entry_by_index(index));
			}

			// Retrieves a sample value from the internal container using `index`.
			// NOTE: Samples are always sorted in ascending order along the 'X' axis.
			const ValueType& get_value_by_index(IndexType index) const
			{
				return std::get<1>(get_entry_by_index(index));
			}

			// Attempts to find the next sample after `key` along the 'X' axis,
			// using the data-subset defined by `begin` and `end`.
			// 
			// If no sample is found, this will return an end-iterator.
			ConstIterator find_next_entry(const KeyType& key, const ConstIterator& begin, const ConstIterator& end) const
			{
				assert(!data.empty());
				assert(begin >= data.begin());
				assert(end <= data.end());

				auto it = std::upper_bound // std::lower_bound
				(
					begin, end,

					key,

					[](const KeyType& key, const Entry& entry)
					{
						//return (key < std::get<0>(entry));
						return (LessImpl {})(key, std::get<0>(entry));
					}
				);

				if (!data.empty())
				{
					if (it == end)
					{
						it = (end - 1);

						assert(it != end);
					}
				}

				return it;
			}

			// Attempts to find the next sample after `key` along the 'X' axis, starting at `offset` into the available data.
			// 
			// If no sample is found, this will return an end-iterator.
			ConstIterator find_next_entry(const KeyType& key, const ConstIterator& offset) const
			{
				return find_next_entry(key, offset, data.cend());
			}

			// Attempts to find the next sample after `key` along the 'X' axis.
			// 
			// If no sample is found, this will return an end-iterator.
			ConstIterator find_next_entry(const KeyType& key) const
			{
				return find_next_entry(key, data.cbegin());
			}

			// Retrieves the next sample after `key` along the 'X' axis.
			const Entry& get_next_entry(const KeyType& key) const
			{
				auto it = find_next_entry(key);

				assert(it != data.cend());

				return *it;
			}

			// Attempts to find the sample located prior `to_it` along the `X` axis.
			// 
			// If there is no prior sample, this will return a copy of `to_it`.
			ConstIterator find_prev_entry(const ConstIterator& to_it) const
			{
				assert(to_it != data.cend());

				if (auto from_it = (to_it - 1); ((from_it != data.cend()) && (from_it >= data.cbegin())))
				{
					return from_it;
				}

				return to_it;
			}

			// Attempts to find the sample located prior to `key` along the 'X' axis.
			// 
			// If there is no prior sample, this will return an iterator to the closest available sample.
			ConstIterator find_prev_entry(const KeyType& key) const
			{
				auto to_it = find_next_entry(key);

				assert(to_it != data.cend());

				return find_prev_entry(to_it);
			}

			// Retrieves a reference to the sample located prior to `to_it`.
			// 
			// If there is no prior sample, this will return a reference to the object pointed to by `to_it`.
			const Entry& get_prev_entry(const ConstIterator& to_it) const
			{
				auto from_it = find_prev_entry(to_it);

				assert(from_it != data.cend());

				return *from_it;
			}

			// Attempts to find the two nearest samples to `key` (lesser and greater).
			// 
			// If a pair could not be found, this will return two end-iterators.
			// 
			// If only one sample could be found (i.e. if `key` is outside of the 'X' axis's scope),
			// this will return the same iterator for both the lesser and greater samples.
			std::pair<ConstIterator, ConstIterator> find_nearest_pair(const KeyType& key) const
			{
				if (auto to_it = find_next_entry(key); to_it != data.cend())
				{
					if (auto from_it = find_prev_entry(to_it); from_it != data.cend())
					{
						return { std::move(from_it), std::move(to_it) };
					}
				}

				return { data.cend(), data.cend() };
			}

			// Retrieves the two nearest samples to `key` (lesser and greater).
			std::pair<const Entry&, const Entry&>
			get_nearest_pair(const KeyType& key) const
			{
				auto [from_it, to_it] = find_nearest_pair(key);

				assert(from_it != data.cend());
				assert(to_it != data.cend());

				return std::make_pair(*from_it, *to_it);
			}

			// Retrieves the next sample value after `key`.
			ValueType get_next_value(const KeyType& key) const
			{
				return std::get<1>(get_next_entry(key));
			}

			// Retrieves a value by sampling on the 'X' axis.
			// 
			// If the `key` specified is beyond the scope of the known
			// 'X' axis, this will return the nearest sample's value.
			// 
			// If `key` is within the bounds of the `X` axis, this will return an
			// interpolated value between the two nearest (lesser and greater) samples.
			ValueType get_value(const KeyType& key) const
			{
				if (data.empty())
				{
					return ValueType {};
				}

				auto [from_it, to_it] = find_nearest_pair(key);

				assert(from_it != data.cend());
				assert(to_it != data.cend());

				const auto& to = *to_it;

				if (from_it == to_it) // ((&from) == (&to))
				{
					return std::get<1>(to);
				}

				const auto& from = *from_it;

				const auto& from_key = std::get<0>(from);
				const auto& to_key = std::get<0>(to);

				const auto from_key_f = static_cast<FloatType>(from_key);

				const auto sample_distance = math::abs(static_cast<FloatType>(to_key) - from_key_f);
				const auto query_distance = (static_cast<FloatType>(key) - from_key_f);

				const auto& to_value = std::get<1>(to);

				if (query_distance >= sample_distance) // (interpolation_ratio >= 1.0f)
				{
					return to_value;
				}

				const auto& from_value = std::get<1>(from);

				const auto interpolation_ratio = (query_distance / sample_distance); // std::min(..., 1.0f);

				const auto result = (LerpImpl {})(from_value, to_value, interpolation_ratio);

				return result;
			}

			// Computes the cumulative sum of sample values from the first element until `key`.
			// 
			// If `allow_interpolation` is enabled and `key` falls between two samples,
			// the 'X-relative' distance between the two samples' values will be added in the final summation.
			// 
			// If `allow_interpolation` is disabled, this will immediately exit upon
			// reaching a `key` that is positioned between two samples.
			// 
			// This in turn means that the final summation will only include the sum of whole sample values,
			// rather than including an interpolated value from the neighboring samples.
			ValueType get_cumulative_value(const KeyType& key, bool allow_interpolation=true) const
			{
				assert(!data.empty());

				auto from_it = data.cbegin();
				auto to_it = (from_it + 1);

				assert(from_it != data.cend());
				assert(to_it != data.cend());

				auto result = std::get<1>(*from_it);

				auto destination_key = static_cast<FloatType>(key);

				while ((to_it != data.cend())) // && (from_it != to_it)
				{
					const auto& from = *from_it;
					const auto& to = *to_it;

					const auto& from_key = std::get<0>(from);
					const auto& to_key = std::get<0>(to);

					const auto sample_offset = static_cast<FloatType>(from_key);
					const auto sample_distance = (static_cast<FloatType>(to_key) - sample_offset);
					
					const auto query_distance = (destination_key - sample_offset);

					const auto& to_value = std::get<1>(to);

					if (query_distance < sample_distance)
					{
						if (allow_interpolation)
						{
							const auto& from_value = std::get<1>(from);

							const auto interpolation_ratio = math::clamp(math::abs((query_distance / sample_distance)), 0.0f, 1.0f);
							const auto interpolation_result = (LerpImpl {})(from_value, to_value, interpolation_ratio);

							result += interpolation_result;
						}

						break;
					}
					else
					{
						result += to_value;
					}

					from_it++; // from_it = to_it;
					to_it++;
				}
				
				return result;
			}

			// Returns a const-reference to the internal container.
			const Container& get_sample_data() const
			{
				return data;
			}

			// Returns a const-iterator to the beginning of the sample data.
			decltype(auto) cbegin() const
			{
				return data.cbegin();
			}

			// Returns a const-iterator to the end of the sample data. (Last element + 1)
			decltype(auto) cend() const
			{
				return data.cend();
			}

			// NOTE: Samplers are immutable, meaning this will always return a const-iterator.
			// Returns a const-iterator to the beginning of the sample data.
			decltype(auto) begin() const
			{
				return cbegin();
			}

			// NOTE: Samplers are immutable, meaning this will always return a const-iterator.
			// Returns a const-iterator to the end of the sample data. (Last element + 1)
			decltype(auto) end() const
			{
				return cend();
			}

			// Indicates true if there are no data-points in this sampler.
			bool empty() const
			{
				return data.empty();
			}

			// Reports the number of data points in this sampler.
			std::size_t size() const
			{
				return data.size();
			}

			// Returns true if this sampler is not empty.
			explicit operator bool() const
			{
				return !empty();
			}

			ValueType operator[](const KeyType& key) const
			{
				return get_value(key);
			}
		protected:
			// Sorts the internal sample data in ascending order, along the 'X' axis.
			void sort()
			{
				std::sort
				(
					data.begin(), data.end(),

					[](const Entry& left, const Entry& right)
					{
						//return (std::get<0>(left) < std::get<0>(right));
						return (LessImpl {})(std::get<0>(left), std::get<0>(right));
					}
				);
			}

			Container data;
	};

	template <typename ValueType>
	using Sampler = SamplerImpl<float, ValueType>;

	using Sampler1D = Sampler<float>;
	using Sampler2D = Sampler<math::Vector2D>;
	using Sampler3D = Sampler<math::Vector3D>;
	using Sampler4D = Sampler<math::Vector4D>;

	using MatrixSampler = Sampler<math::Matrix>;
	using RotationMatrixSampler = Sampler<math::RotationMatrix>;
	using QuaternionSampler = Sampler<math::Quaternion>;

	using MatSampler = MatrixSampler;
	using RMatSampler = RotationMatrixSampler;
	using QuatSampler = QuaternionSampler;

	using FloatSampler = Sampler1D;
}