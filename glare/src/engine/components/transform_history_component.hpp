#pragma once

//#include "transform_component.hpp"

#include <engine/types.hpp>
#include <math/types.hpp>
#include <util/small_vector.hpp>

namespace engine
{
	struct Transform;

	// When attached to an entity that also has a `TransformComponent`,
	// this will reflect the previous frame/update tick's `TransformComponent` value.
	// 
	// TODO: Optimize this by only storing Position and Rotation, rather than all three.
	// TODO: Look into pros/cons of switching to a matrix-based approach.
	// TODO: Look into refactoring this type into an optimized deque-like structure (based on `small_vector`) for general use.
	struct TransformHistoryComponent
	{
		public:
			// The current default is 2 frames pre-allocated. (2 x 3x3x4 bytes + overhead, currently)
			static constexpr std::size_t PREALLOCATED_ENTRIES = 2;

			using OptimizedTransform = math::TransformVectors;

			// NOTE: We use a `small_vector` here since `std::deque` would require heap storage.
			using TransformHistory   = util::small_vector<OptimizedTransform, PREALLOCATED_ENTRIES>;

			TransformHistoryComponent() = default;
			TransformHistoryComponent(const TransformHistoryComponent&) = default;
			TransformHistoryComponent(TransformHistoryComponent&&) noexcept = default;

			TransformHistoryComponent(TransformHistory&& previous) noexcept;
			TransformHistoryComponent(OptimizedTransform&& prev_tform, bool force_populate_history=false);
			TransformHistoryComponent(const Transform& tform);

			TransformHistoryComponent& operator=(const TransformHistoryComponent&) = default;
			TransformHistoryComponent& operator=(TransformHistoryComponent&&) noexcept = default;

			// Retrieves a copy of the latest transformation snapshot.
			OptimizedTransform get_prev_vectors() const;

			// Retrieves the last known transform position.
			math::Vector get_prev_position() const;

			// Retrieves the last known transform rotation.
			math::Vector get_prev_rotation() const;

			// Retrieves the last known transform scale.
			math::Vector get_prev_scale() const;

			// Computes the average 'position' value over the specified number of samples.
			math::Vector get_avg_position(std::size_t n_samples, std::size_t offset=0) const;

			// Computes the average 'rotation' value over the specified number of samples.
			math::Vector get_avg_rotation(std::size_t n_samples, std::size_t offset=0) const;

			// Computes the average 'scale' value over the specified number of samples.
			math::Vector get_avg_scale(std::size_t n_samples, std::size_t offset=0) const;

			// Indicates the number of transform samples available.
			std::size_t size() const;

			// Indicates the maximum number of transform samples that can be held at one time.
			// (Without manually trying to expand; see `expand_history` parameter of `push_snapshot`)
			std::size_t capacity() const;

			// Adds `tform` to the start of the internal history collection.
			// 
			// If `expand_history` is enabled, this routine may allocate memory
			// to preserve the oldest entries in the internal history collection.
			void push_snapshot(const OptimizedTransform& tform, bool expand_history=false);

			// Convenience overload for handling `Transform` objects.
			void push_snapshot(const Transform& tform, bool expand_history=false);

			// Convenience operator for `push_snapshot`.
			inline TransformHistoryComponent& operator<<(const OptimizedTransform& tform)
			{
				push_snapshot(tform, false);

				return *this;
			}

			inline TransformHistoryComponent& operator<<(const Transform& tform)
			{
				push_snapshot(tform, false);

				return *this;
			}

			inline const TransformHistory& get_history() const { return previous; }

			// TODO: Determine if this should be part of the public API.
			//inline TransformHistory& get_history() { return previous; }
		protected:
			// NOTE: This may change to a protected/internal function later.
			// If unsure, use `get_prev_vectors` instead.
			const OptimizedTransform& peek_snapshot() const;

			// TODO: Determine if this should be part of the public API.
			// Clears the internal transform history.
			void clear();

			// Flushes the oldest entry from `previous`,
			// shifting all previous elements forward by one index.
			// 
			// NOTE: This leaves the leftmost element(s) in an unspecified (moved-from) state.
			void flush();

			// A collection of previous transformation states.
			TransformHistory previous;
	};
}