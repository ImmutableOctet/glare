#pragma once

#include "types.hpp"

#include <util/type_traits.hpp>

#include <chrono>
#include <optional>
#include <string_view>
#include <type_traits>

namespace engine
{
	// Basic high-precision timer abstraction using `std::chrono`.
	class Timer
	{
		public:
			using Clock        = std::chrono::system_clock;
			using TimePoint    = std::chrono::time_point<Clock>; // Clock::time_point;
			using Duration     = Clock::duration;
			using DurationRaw  = Duration::rep;

			using Days         = std::chrono::days;
			using Hours        = std::chrono::hours;
			using Minutes      = std::chrono::minutes;
			using Seconds      = std::chrono::seconds; 
			using Milliseconds = std::chrono::milliseconds;
			using Microseconds = std::chrono::microseconds;

			using FloatSeconds = std::chrono::duration<float>;
			using DoubleSeconds = std::chrono::duration<double>;

			template <typename T>
			inline static constexpr bool IsDurationType = util::is_specialization_v<T, std::chrono::duration>;

			static std::optional<Duration> to_duration(DurationRaw duration_rep, StringHash time_symbol_id);
			static std::optional<Duration> to_duration(DurationRaw duration_rep, std::string_view time_symbol);

			static std::optional<Duration> to_duration(float duration_raw, StringHash time_symbol_id);
			static std::optional<Duration> to_duration(double duration_raw, StringHash time_symbol_id);

			static std::optional<Duration> to_duration(double duration_raw, std::string_view time_symbol);
			static std::optional<Duration> to_duration(float duration_raw, std::string_view time_symbol);

			static Duration to_duration(FloatSeconds seconds);
			static Duration to_duration(float seconds);

			static Duration to_duration(DoubleSeconds seconds);
			static Duration to_duration(double seconds);

			template
			<
				typename DurationType,

				std::enable_if<IsDurationType<DurationType>, int>::type=0
			>
			static Duration to_duration(DurationType duration_value)
			{
				return std::chrono::duration_cast<Duration>(duration_value);
			}

			// Default initializes a timer with no specified length/duration.
			Timer() = default;

			// Initializes a timer with the specified `length`.
			//
			// If `start_immediately` is true, this will immediately call `start`
			// using the newly initialized `length` value.
			Timer(Duration length, bool start_immediately=true);

			template
			<
				typename DurationType,

				std::enable_if<IsDurationType<DurationType>, int>::type=0
			>
			Timer(DurationType duration_value, bool start_immediately=true)
				: Timer(to_duration(duration_value), start_immediately) {}

			//virtual ~Timer();

			// Starts the timer at the current point in time.
			//
			// IF the timer is currently paused and `restart` is false,
			// this will forward to the `resume` method instead.
			//
			// If `restart` is true, this will clear any existing (paused) progress and start over.
			bool start(bool restart=false);

			// Force starts this timer with the length specified.
			// 
			// If this timer has already been initialized with the correct length/duration,
			// feel free to use the (default) parameterless version of `start` or `restart`.
			bool start(Duration length);

			// Alias to `start` with `restart` set to true.
			bool restart();

			// Alias to `start` overload taking a length/duration.
			bool restart(Duration length);

			// Pauses the timer at the current point in time.
			//
			// A paused timer can be resumed using the `resume` method, or by calling `start` with no arguments specified.
			bool pause();

			// Resumes a paused timer, updating the projected end-point in the process.
			//
			// A paused timer can be resumed using this method directly, or by calling `start` with no arguments specified.
			bool resume();

			// Stops this timer immediately, clearing all progress made.
			// 
			// The last specified length/duration is preserved after calling
			// this method, but all previously established time-points are cleared.
			//
			// See also: `clear`
			bool stop();

			// Stops this timer, then clears the length/duration.
			//
			// The return value of this method indicates if a `stop` operation needed to be performed.
			bool clear();

			// Sets a new length/duration for this timer.
			bool set_duration(Duration length);

			// Indicates if this timer has been started.
			//
			// NOTE:
			// A started timer may also be `paused`.
			// To check if a timer is currently counting down (i.e. active),
			// use the `active` method instead.
			bool started() const;

			// Indicates if this timer is currently paused.
			bool paused() const;

			// Indicates whether a projected end-point currently exists.
			//
			// See also: `get_projected_end_point`
			bool has_projected_end() const;

			// Checks if a timer is currently active.
			// (Started, but not paused)
			bool active() const;

			// Indicates if this timer is currently stopped.
			// (i.e. not yet started)
			//
			// See also: `started`, `paused`, `active`
			bool stopped() const;

			// Checks whether `remaining` indicates a duration of
			// zero or less than zero, thus indicating completion.
			//
			// If this timer is currently stopped, this will always return false.
			bool completed() const;

			// Indicates the last established length/duration for this timer.
			// 
			// If this timer is started from the beginning (or restarted), this
			// will be the duration the timer will be active for.
			Duration duration() const;

			// Indicates the `duration` of this timer in milliseconds.
			Milliseconds milliseconds() const;

			// Indicates the amount of time elapsed since this timer's start-point.
			// 
			// If this timer is currently `stopped`, this will indicate an empty duration.
			// 
			// If this timer is currently paused, this will indicate the length
			// in time that this timer ran for, before becoming paused.
			Duration elapsed() const;

			// Indicates the `elapsed` time of this timer in milliseconds.
			Milliseconds milliseconds_elapsed() const;

			// Indicates the remaining amount of time needed for this timer to be considered `completed`.
			//
			// If this timer is `stopped`, this will return the `duration`.
			// 
			// If this timer is `paused`, this will return the
			// difference between the time `elapsed` and the established `duration`.
			//
			// If this timer is `active` (i.e. `started` and not `paused`), this will
			// return the amount of time between now and the projected end-point.
			Duration remaining() const;

			// Indicates the `remaining` time of this timer in milliseconds.
			Milliseconds milliseconds_remaining() const;

			// Retrieves the last established start-point.
			// 
			// This point is updated when starting, restarting, or resuming this timer.
			// NOTE: The value of this field is preserved while this timer is paused. (Updated on resume)
			std::optional<TimePoint> get_start_point() const;

			// Retrieves the current pause-point. If this timer isn't
			// currently paused, this will return `std::nullopt`.
			std::optional<TimePoint> get_pause_point() const;

			// Indicates the last known projected end-point.
			// 
			// This is updated when stopping, starting or resuming, but is otherwise preserved.
			// (e.g. the last projected end-point is preserved while paused, but later updated when resuming)
			std::optional<TimePoint> get_projected_end_point() const;

			// Simple alias to default (parameterless) overload of `start`.
			inline bool activate()
			{
				return start();
			}

			// Alias for `active` method.
			inline explicit operator bool() const
			{
				return ((active()) && (!completed()));
			}

			// Equivalent to calling `activate`, or `start` with no parameters.
			inline bool operator()()
			{
				return activate();
			}

			friend auto operator<=>(const Timer&, const Timer&) = default;
		protected:
			static constexpr Duration to_duration_seconds_impl(auto seconds)
			{
				return std::chrono::duration_cast<Duration>(std::chrono::round<std::chrono::nanoseconds>(seconds));
			}

			std::optional<TimePoint> start_point;
			std::optional<TimePoint> pause_point;
			std::optional<TimePoint> end_point;

			Duration length = {};
	};
}