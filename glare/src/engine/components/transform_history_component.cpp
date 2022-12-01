#include "transform_history_component.hpp"

#include <engine/transform.hpp>
#include <math/math.hpp>
#include <util/algorithm.hpp>

#include <algorithm>
#include <utility>
#include <tuple>

namespace engine
{
	static constexpr auto generate_default_transform()
	{
		return TransformHistoryComponent::OptimizedTransform { {}, {}, { 1.0f, 1.0f, 1.0f } };
	}

	template <std::size_t tuple_index>
	static auto get_avg_value
	(
		const TransformHistoryComponent::TransformHistory& previous,
		std::size_t n_samples,
		std::size_t offset=0
	)
	{
		return util::get_avg_value_from_tuple_samples<tuple_index>
		(
			previous, n_samples,
			
			[](const math::Vector3D& value, std::size_t divisor) -> math::Vector3D
			{
				auto divisor_f = static_cast<float>(divisor);

				//return { (value.x / divisor_f), (value.y / divisor_f), (value.z / divisor_f) };

				return (value / divisor_f);
			},

			offset
		);
	}

	TransformHistoryComponent::TransformHistoryComponent()
		: TransformHistoryComponent(generate_default_transform(), false) // true
	{}

	TransformHistoryComponent::TransformHistoryComponent(TransformHistory&& previous) noexcept
		: previous(std::move(previous)) {}

	TransformHistoryComponent::TransformHistoryComponent(OptimizedTransform&& prev_tform, bool force_populate_history)
	{
		previous.reserve(PREALLOCATED_ENTRIES);

		if (force_populate_history)
		{
			// Generate copies of `prev_tform` to fill the previous entry slots.
			for (std::size_t i = 1; i <= (PREALLOCATED_ENTRIES - 1); i++)
			{
				previous.emplace_back(prev_tform);
			}
		}

		// Move `prev_tform` into the latest slot.
		previous.emplace_back(std::move(prev_tform));
	}

	TransformHistoryComponent::TransformHistoryComponent(const Transform& tform)
		: TransformHistoryComponent(tform.get_vectors()) {}

	const TransformHistoryComponent::OptimizedTransform& TransformHistoryComponent::peek_snapshot() const
	{
		return previous[0];
	}

	TransformHistoryComponent::OptimizedTransform TransformHistoryComponent::get_prev_vectors() const // math::TransformVectors
	{
		//assert(!previous.empty());

		if (previous.empty())
		{
			return generate_default_transform();
		}

		// No type conversion necessary. (may change later)
		return peek_snapshot();
	}

	math::Vector TransformHistoryComponent::get_prev_position() const
	{
		return std::get<0>(get_prev_vectors()); // peek_snapshot()
	}

	math::Vector TransformHistoryComponent::get_prev_rotation() const
	{
		return std::get<1>(get_prev_vectors()); // peek_snapshot()
	}

	math::Vector TransformHistoryComponent::get_prev_scale() const
	{
		return std::get<2>(get_prev_vectors()); // peek_snapshot()
	}

	math::Vector TransformHistoryComponent::get_avg_position(std::size_t n_samples, std::size_t offset) const
	{
		return get_avg_value<0>(previous, n_samples, offset);
	}

	math::Vector TransformHistoryComponent::get_avg_rotation(std::size_t n_samples, std::size_t offset) const
	{
		return get_avg_value<1>(previous, n_samples, offset);
	}

	math::Vector TransformHistoryComponent::get_avg_scale(std::size_t n_samples, std::size_t offset) const
	{
		return get_avg_value<2>(previous, n_samples, offset);
	}

	std::size_t TransformHistoryComponent::size() const
	{
		return previous.size();
	}

	std::size_t TransformHistoryComponent::capacity() const
	{
		return previous.capacity();
	}

	void TransformHistoryComponent::push_snapshot(const OptimizedTransform& tform, bool expand_history)
	{
		if (expand_history)
		{
			// Add `tform` to the beginning of the collection,
			// maintaining all previous entries.
			previous.insert(previous.begin(), tform);
		}
		else
		{
			//previous.erase(v.end() - 1);
			//previous.insert(v.begin(), tform);

			// Move previous entries over by one,
			// truncating the oldest if necessary.
			flush();

			// Utilize the (now-empty) first index by populating it with `tform`.
			previous[0] = tform;
		}
	}

	void TransformHistoryComponent::push_snapshot(const Transform& tform, bool expand_history)
	{
		push_snapshot(tform.get_vectors(), expand_history);
	}

	void TransformHistoryComponent::clear()
	{
		previous.clear();
	}

	void TransformHistoryComponent::flush()
	{
		if (previous.empty())
		{
			return;
		}

		// Shift right by one element.
		std::move(previous.begin(), previous.end(), previous.begin() + 1);
		// Alternative: std::shift_right(previous.begin(), previous.end(), 1);
	}
}