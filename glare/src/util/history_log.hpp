#pragma once

//#include "small_vector.hpp"

#include <vector>
#include <utility>
#include <type_traits>

//#include <cstdint>
#include <cassert>

namespace util
{
	/*
		Stores, restores and seeks through a collection of immutable 'snapshots' of type `T`.

		The `T` argument must be either copy-constructible or move-constructible.
		API availability and usage must align accordingly.
	*/
	template
	<
		typename T,
		typename ContainerType=std::vector<T>, // util::small_vector<T, 8>
		typename SizeType=std::size_t,
		typename CursorType=SizeType
	>
	class HistoryLog
	{
		public:
			// General-purpose type aliases:
			using Snapshot = T;
			using SnapshotContainer = ContainerType;
			using SnapshotCursor = CursorType;

			// STL compatibility:
			using value_type = T;
			using size_type = SizeType;
			using const_reference = const T&;
			using reference = const_reference; // const T&;

		protected:
			//inline static constexpr bool copy_assign_enabled = (std::is_copy_assignable_v<T>);
			//static_assert(copy_assign_enabled);

			inline static constexpr bool move_enabled = (std::is_move_constructible_v<T>); // || (std::is_default_constructible_v<T> && std::is_move_assignable_v<T>);
			inline static constexpr bool copy_enabled = (std::is_copy_constructible_v<T>); // || (std::is_default_constructible_v<T> && std::is_copy_assignable_v<T>); // (... && copy_assign_enabled);

			static_assert((move_enabled || copy_enabled));

			template <typename Input, typename Output>
			static bool apply_snapshot(Input&& input, Output&& output)
			{
				if constexpr (std::is_invocable_r_v<bool, Output, Input>)
				{
					return output(std::forward<Input>(input));
				}
				else if constexpr (std::is_invocable_v<Output, Input>)
				{
					output(std::forward<Input>(input));

					return true;
				}
				else if constexpr (std::is_invocable_r_v<bool, Output>)
				{
					return output();
				}
				else if constexpr (std::is_invocable_v<Output>)
				{
					output();

					return true;
				}
				else if constexpr (std::is_lvalue_reference_v<Output> && (!std::is_const_v<std::remove_reference_t<Output>>)) // && std::is_same_v<std::decay_t<Output>, std::decay_t<T>>
				{
					static_assert(std::integral_constant<bool, std::is_assignable_v<Output, Input>>::value, "The output type must be copy-assignable from a reference to `T`.");

					output = std::forward<Input>(input);

					return true;
				}
				else
				{
					return false;
				}
			}

		public:
			static consteval SnapshotCursor default_cursor()
			{
				return SnapshotCursor {};
			}

			static bool cursor_is_default(const SnapshotCursor& cursor)
			{
				return (cursor == default_cursor());
			}

			/*
				Attempts to restore a previous state of `T` by passing it to `output`, adjusting the `cursor` accordingly.
				
				* The `output` argument may be a callable taking an instance of `T` (or `const T&`) as its only parameter,
				or it may be a non-const lvalue-reference to an object that is capable of copy-assignment from `T`.

				* The `on_first_undo` argument must be a callable taking either no arguments, or a single lvale-reference to this object as input.
				
				`on_first_undo` is executed when this log reports true for `can_undo` (i.e. historical data is available),
				and the `cursor` field points to the live state of `T` (see `live_value_cursor`).
				
				The return value of `on_first_undo` indicates if the `undo` operation shall continue.
				If no return value is provided, the operation will continue as expected.
			*/
			template <typename Output, typename FirstUndoCallback>
			bool undo(Output&& output, FirstUndoCallback&& on_first_undo)
			{
				if (cursor_points_to_live_value())
				{
					if (can_undo())
					{
						if constexpr (std::is_invocable_r_v<bool, FirstUndoCallback, decltype(*this)>)
						{
							if (!on_first_undo(*this))
							{
								return false;
							}
						}
						else if constexpr (std::is_invocable_v<FirstUndoCallback, decltype(*this)>)
						{
							on_first_undo(*this);
						}
						else if constexpr (std::is_invocable_r_v<bool, FirstUndoCallback>)
						{
							if (!on_first_undo())
							{
								return false;
							}
						}
						else
						{
							on_first_undo();
						}
					}
					else
					{
						return false;
					}
				}

				// NOTE: Regular control paths may result in multiple calls to `can_undo`.
				// This is intentional, as `on_first_undo` may change the state of this log.
				if (!can_undo())
				{
					return false;
				}

				assert(cursor > 0);

				const auto prev_cursor = (cursor - 1);

				const auto prev_snapshot = get_snapshot(static_cast<SizeType>(prev_cursor));

				assert(prev_snapshot);

				if (!prev_snapshot)
				{
					return false;
				}

				if (!apply_snapshot(*prev_snapshot, std::forward<Output>(output)))
				{
					return false;
				}
				
				cursor = prev_cursor;

				return true;
			}

			// This is a small forwarding overload for `undo`,
			// allowing for the omission of the `on_first_undo` argument.
			// 
			// For more details on `undo`, please see the primary overload.
			template <typename Output>
			bool undo(Output&& output)
			{
				return undo
				(
					std::forward<Output>(output),

					[]() { /* Empty implementation. */ }
				);
			}

			// Returns a (temporary) const-pointer to an instance of
			// `T` retrieved during the `undo` operation.
			const T* undo()
			{
				const T* output = nullptr;

				undo
				(
					[&output](const auto& value)
					{
						output = &value;
					}
				);

				return output;
			}

			/*
				Attempts to reinstate a snapshot of `T` by passing it to `output`, adjusting the `cursor` accordingly.

				* The `output` argument may be a callable taking an instance of `T` (or `const T&`) as its only parameter,
				or it may be a non-const lvalue-reference to an object that is capable of copy-assignment from `T`.

				* The `on_last_redo` argument must be a callable taking either no arguments, or a const lvalue-reference to this object.
				
				`on_last_redo` is executed when the next state `cursor` will point to is the live state of `T`.
				(i.e. `cursor` would be equal to `live_value_cursor` after success of this `redo` operation)

				The return value of `on_last_redo` indicates if the `redo` operation shall continue.
				If no return value is provided, the operation will continue as expected.
			*/
			template <typename Output, typename OnLastRedo>
			bool redo(Output&& output, OnLastRedo&& on_last_redo)
			{
				if (!can_redo())
				{
					return false;
				}

				const auto next_cursor = (cursor + 1);

				if (next_cursor == live_value_cursor())
				{
					if constexpr (std::is_invocable_r_v<bool, OnLastRedo, decltype(std::as_const(*this))>)
					{
						if (!on_last_redo(std::as_const(*this)))
						{
							return false;
						}
					}
					else if constexpr (std::is_invocable_v<OnLastRedo, decltype(std::as_const(*this))>)
					{
						on_last_redo(std::as_const(*this));
					}
					else if constexpr (std::is_invocable_r_v<bool, OnLastRedo>)
					{
						if (!on_last_redo())
						{
							return false;
						}
					}
					else
					{
						on_last_redo();
					}
				}

				const auto active_snapshot = get_active_snapshot(); // get_snapshot(static_cast<SizeType>(cursor));

				assert(active_snapshot);

				if (!active_snapshot)
				{
					return false;
				}

				if (!apply_snapshot(*active_snapshot, std::forward<Output>(output)))
				{
					return false;
				}

				cursor = next_cursor;

				return true;
			}

			// This is a small forwarding overload for `redo`,
			// allowing for the omission of the `on_last_redo` argument.
			// 
			// For more details on `redo`, please see the primary overload.
			template <typename Output>
			bool redo(Output&& output)
			{
				return redo
				(
					std::forward<Output>(output),

					[](){}
				);
			}

			// Returns a (temporary) const-pointer to an instance of
			// `T` retrieved during the `redo` operation.
			const T* redo()
			{
				const T* output = nullptr;

				redo
				(
					[&output](const auto& value)
					{
						output = &value;
					}
				);

				return output;
			}

			// Takes a snapshot, using `value` as its contents.
			std::enable_if_t<move_enabled, bool> store(T&& value)
			{
				return store_impl(std::move(value));
			}

			// Takes a snapshot, using a copy of `value` as its contents.
			std::enable_if_t<copy_enabled, bool> store(const T& value)
			{
				return store_impl(value);
			}

			// Truncates the internal `snapshots` collection to the current position of `cursor`.
			// (i.e. erases all historical data from `cursor` onward)
			bool truncate()
			{
				if (!can_truncate())
				{
					return false;
				}

				snapshots.resize(static_cast<SizeType>(cursor));

				return true;
			}

			// Clears the contents of this log.
			bool clear()
			{
				if (!can_clear())
				{
					//assert(has_default_cursor());

					return false;
				}

				snapshots.clear();
				cursor = {};

				return true;
			}

			// Returns true if truncation can be performed.
			bool can_truncate() const
			{
				const auto intended_cutoff = (static_cast<SizeType>(cursor) + static_cast<SizeType>(1));

				return (intended_cutoff <= size());
			}

			// Returns true if a call to `undo` can succeed.
			bool can_undo() const
			{
				return ((cursor > 0) && (cursor <= size()));
			}

			// Returns true if a call to `redo` can succeed.
			bool can_redo() const
			{
				return (cursor < size()); // (!cursor_out_of_bounds((cursor + 1)));
			}

			// Indicates if a `clear` operation can be performed.
			// NOTE: Should always return true.
			bool can_clear() const
			{
				return (!snapshots.empty());
			}

			// Indicates if a `store` operation can be performed.
			// NOTE: Should always return true.
			bool can_store() const
			{
				return true;
			}

			// Returns true if the `cursor` specified does not point to an available snapshot.
			bool cursor_out_of_bounds(const SnapshotCursor& cursor) const
			{
				return (static_cast<SizeType>(cursor) >= size());
			}

			// Returns true if `cursor` is in its default state.
			// (i.e. equal to zero)
			bool has_default_cursor() const
			{
				return cursor_is_default(cursor);
			}

			// Returns true if the `cursor` field points to `live_value_cursor`,
			// representing the latest state of `T`.
			// 
			// (i.e. there are no newer snapshots known by this log.)
			bool has_live_value_cursor() const
			{
				return (cursor == live_value_cursor()); // size();
			}

			// Alias for `has_live_value_cursor`.
			bool cursor_points_to_live_value() const
			{
				return has_live_value_cursor();
			}

			// Returns true if the underlying snapshot type can be moved.
			constexpr bool can_move_value() const
			{
				return move_enabled;
			}

			// Returns true if the underlying snapshot type can be copied.
			constexpr bool can_copy_value() const
			{
				return copy_enabled;
			}

			// Retrieves a temporary const-reference to the `cursor` field.
			const SnapshotCursor& get_cursor() const
			{
				return cursor;
			}

			// Retrieves a temporary const-reference to the `snapshots` field.
			const SnapshotContainer& get_snapshots() const
			{
				return snapshots;
			}

			// Returns the size of the `snapshots` container.
			SizeType size() const
			{
				return static_cast<SizeType>(snapshots.size());
			}

			// Returns true if `snapshots` does not contain any values.
			bool empty() const
			{
				return snapshots.empty();
			}

			// Returns a value representing the live instance of `T`.
			// (Equivalent to `size`)
			SnapshotCursor live_value_cursor() const
			{
				return static_cast<SnapshotCursor>(size());
			}

			/*
				Retrieves a (temporary) const-pointer to the snapshot identified by `index`.

				If `index` does not point at a valid snapshot, or if `index`
				points to `live_value_cursor` (i.e. `size`), this will return `nullptr`.
			*/
			const T* get_snapshot(SizeType index) const
			{
				if (index >= size())
				{
					return nullptr;
				}

				return &(snapshots[static_cast<std::size_t>(index)]);
			}

			/*
				Retrieves a (temporary) const-pointer to the snapshot identified by `cursor`.
				
				If `cursor` is not currently pointing at a valid snapshot, or if
				`cursor` points to `live_value_cursor`, this will return `nullptr`.
			*/
			const T* get_active_snapshot() const
			{
				return get_snapshot(static_cast<SizeType>(cursor));
			}

			// Returns a const-iterator representing the beginning of `snapshots`.
			auto cbegin() const
			{
				return snapshots.cbegin();
			}

			// This acts as an alias to the `cbegin` member-function.
			// External mutation of `snapshots` is not allowed.
			auto begin() const
			{
				return cbegin();
			}

			// Returns a const-iterator representing the end of `snapshots`.
			auto cend() const
			{
				return snapshots.cend();
			}

			// This acts as an alias to the `cend` member-function.
			// External mutation of `snapshots` is not allowed.
			auto end() const
			{
				return cend();
			}

			// Equivalent to calling `can_undo`.
			explicit operator bool() const
			{
				return can_undo();
			}

			// Equivalent to calling `store`.
			template <typename ValueRef>
			HistoryLog& operator<<(ValueRef&& value)
			{
				store(std::forward<ValueRef>(value));

				return *this;
			}

			// Equivalent to calling `undo`.
			template <typename Output>
			HistoryLog& operator>>(Output&& output)
			{
				if (!undo(std::forward<Output>(output)))
				{
					if constexpr (((std::is_default_constructible_v<Output>) && (std::is_copy_assignable_v<Output> || std::is_move_assignable_v<Output>)))
					{
						output = std::decay_t<Output> {};
					}
				}

				return *this;
			}

		private:
			// Internal subroutine; use `store` instead.
			template <typename ValueRef>
			bool store_impl(ValueRef&& value)
			{
				if (!can_store())
				{
					return false;
				}

				truncate();

				if (!emplace(std::forward<ValueRef>(value)))
				{
					return false;
				}

				cursor = live_value_cursor();

				return true;
			}

		protected:
			// Emplaces `value` in `snapshots`, but does not
			// truncate or alter the `cursor` in any way.
			// 
			// See also: `store`
			template <typename ValueRef>
			bool emplace(ValueRef&& value)
			{
				snapshots.emplace_back(std::forward<ValueRef>(value));

				return true;
			}

			/*
				A 'cursor' (i.e. index) into `snapshots`, representing the active state of a `T` object.
				
				If `cursor` is equal to `live_value_cursor` (i.e. `size`),
				then `cursor` may not actually point to a valid index of `snapshots`,
				as this state normally symbolizes that there are no newer historical snapshots to read from.
			*/
			SnapshotCursor cursor = default_cursor();

			// A collection of immutable copies representing previous states of a `T` object.
			SnapshotContainer snapshots;
	};
}