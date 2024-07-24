#pragma once

#include "types.hpp"

#include "time_decl.hpp"

#include <chrono>
#include <optional>
#include <type_traits>

namespace engine
{
	namespace impl
	{
		// High-precision timer abstraction based on system clock.
		template <typename ClockType, typename TimePointType, typename DurationType>
		class TimerImpl
		{
			public:
				using Clock         = ClockType;
				using TimePoint     = TimePointType;

				using Duration      = DurationType;
				using DurationRaw   = typename Duration::rep;

				using Days          = time::shared::Days;
				using Hours         = time::shared::Hours;
				using Minutes       = time::shared::Minutes;
				using Seconds       = time::shared::Seconds; 
				using Milliseconds  = time::shared::Milliseconds;
				using Microseconds  = time::shared::Microseconds;

				using FloatSeconds  = time::shared::FloatSeconds;
				using DoubleSeconds = time::shared::DoubleSeconds;

				template <typename T>
				inline static constexpr bool IsDurationType = engine::time::shared::IsDurationType<T>;

				// Default initializes a timer with no specified length/duration.
				TimerImpl() = default;

				// Initializes a timer with the specified `length`.
				//
				// If `start_immediately` is true, this will immediately call `start`
				// using the newly initialized `length` value.
				TimerImpl(Duration length, bool start_immediately=true) :
					length(std::move(length))
				{
					if (start_immediately)
					{
						start();
					}
				}

				template
				<
					typename DurationType,
					typename ToDurationFn,

					std::enable_if<IsDurationType<DurationType>, int>::type=0
				>
				TimerImpl(DurationType duration_value, ToDurationFn&& to_duration, bool start_immediately=true)
					: TimerImpl(to_duration(duration_value), start_immediately) {}

				//virtual ~TimerImpl();

				// Starts the timer at the current point in time.
				//
				// IF the timer is currently paused and `restart` is false,
				// this will forward to the `resume` method instead.
				//
				// If `restart` is true, this will clear any existing (paused) progress and start over.
				bool start(bool restart=false)
				{
					if (!restart)
					{
						if (paused())
						{
							return resume();
						}

						if (started())
						{
							return false;
						}
					}

					const auto now = Clock::now();

					start_point = now;
					pause_point = std::nullopt;
					end_point   = (now + length);

					return true; // started();
				}

				// Force starts this timer with the length specified.
				// 
				// If this timer has already been initialized with the correct length/duration,
				// feel free to use the (default) parameterless version of `start` or `restart`.
				bool start(Duration length)
				{
					if (started())
					{
						return false;
					}

					this->length = length;

					return start(true);
				}

				// Alias to `start` with `restart` set to true.
				bool restart()
				{
					return start(true);
				}

				// Alias to `start` overload taking a length/duration.
				bool restart(Duration length)
				{
					return start(length);
				}

				// Pauses the timer at the current point in time.
				//
				// A paused timer can be resumed using the `resume` method, or by calling `start` with no arguments specified.
				bool pause()
				{
					if (paused())
					{
						return false;
					}

					pause_point = Clock::now();

					return paused();
				}

				// Resumes a paused timer, updating the projected end-point in the process.
				//
				// A paused timer can be resumed using this method directly, or by calling `start` with no arguments specified.
				bool resume()
				{
					if (!paused())
					{
						return false;
					}

					const auto now = Clock::now();

					const auto prev_elapsed = (start_point) ? (*pause_point - *start_point) : Duration {};
					const auto remaining_length = (length - prev_elapsed);

					pause_point = std::nullopt;
					start_point = now;
					end_point   = (now + remaining_length);

					return true;
				}

				// Stops this timer immediately, clearing all progress made.
				// 
				// The last specified length/duration is preserved after calling
				// this method, but all previously established time-points are cleared.
				//
				// See also: `clear`
				bool stop()
				{
					if (stopped())
					{
						return false;
					}

					start_point = std::nullopt;
					pause_point = std::nullopt;
					end_point   = std::nullopt;

					return true; // stopped();
				}

				// Stops this timer, then clears the length/duration.
				//
				// The return value of this method indicates if a `stop` operation needed to be performed.
				bool clear()
				{
					if (stop())
					{
						length = {};

						return true;
					}

					return false;
				}

				// Sets a new length/duration for this timer.
				bool set_duration(Duration length)
				{
					this->length = length;

					if (paused())
					{
						const auto now = Clock::now();

						start_point = now; // std::nullopt;
						pause_point = now;

						// Projected end point updated for accuracy when comparing against new length.
						// (Practically speaking, this shouldn't matter for start/stop/pause behavior)
						end_point   = (now + length);

						return true;
					}

					if (started())
					{
						// Restart with new length.
						return start(length);
					}

					// Not yet started, just the length assignment will do.
					return true;
				}

				// Indicates if this timer has been started.
				//
				// NOTE:
				// A started timer may also be `paused`.
				// To check if a timer is currently counting down (i.e. active),
				// use the `active` method instead.
				bool started() const
				{
					return start_point.has_value();
				}

				// Indicates if this timer is currently paused.
				bool paused() const
				{
					return pause_point.has_value();
				}

				// Indicates whether a projected end-point currently exists.
				//
				// See also: `get_projected_end_point`
				bool has_projected_end() const
				{
					return end_point.has_value();
				}

				// Checks if a timer is currently active.
				// (Started, but not paused)
				bool active() const
				{
					return (started() && !paused());
				}

				// Indicates if this timer is currently stopped.
				// (i.e. not yet started)
				//
				// See also: `started`, `paused`, `active`
				bool stopped() const
				{
					return (!started());
				}

				// Checks whether `remaining` indicates a duration of
				// zero or less than zero, thus indicating completion.
				//
				// If this timer is currently stopped, this will always return false.
				bool completed() const
				{
					if (stopped())
					{
						return false;
					}

					return (remaining() <= Duration(0));
				}

				// Indicates the last established length/duration for this timer.
				// 
				// If this timer is started from the beginning (or restarted), this
				// will be the duration the timer will be active for.
				Duration duration() const
				{
					return length;
				}

				// Indicates the `duration` of this timer in milliseconds.
				Milliseconds milliseconds() const
				{
					return std::chrono::duration_cast<Milliseconds>(duration());
				}

				// Indicates the amount of time elapsed since this timer's start-point.
				// 
				// If this timer is currently `stopped`, this will indicate an empty duration.
				// 
				// If this timer is currently paused, this will indicate the length
				// in time that this timer ran for, before becoming paused.
				Duration elapsed() const
				{
					if (!has_projected_end())
					{
						return {};
					}

					if (paused())
					{
						//return (*end_point - *pause_point);
						return (*pause_point - *start_point);
					}

					const auto now = Clock::now();

					return (now - *start_point); // *end_point - ...
				}

				// Indicates the `elapsed` time of this timer in milliseconds.
				Milliseconds milliseconds_elapsed() const
				{
					return std::chrono::duration_cast<Milliseconds>(elapsed());
				}

				// Indicates the remaining amount of time needed for this timer to be considered `completed`.
				//
				// If this timer is `stopped`, this will return the `duration`.
				// 
				// If this timer is `paused`, this will return the
				// difference between the time `elapsed` and the established `duration`.
				//
				// If this timer is `active` (i.e. `started` and not `paused`), this will
				// return the amount of time between now and the projected end-point.
				Duration remaining() const
				{
					//return (length - elapsed());

					if (!has_projected_end())
					{
						return length;
					}

					if (paused())
					{
						return (length - elapsed());
					}

					const auto now = Clock::now();

					return (*end_point - now);
				}

				// Indicates the `remaining` time of this timer in milliseconds.
				Milliseconds milliseconds_remaining() const
				{
					return std::chrono::duration_cast<Milliseconds>(remaining());
				}

				// Retrieves the last established start-point.
				// 
				// This point is updated when starting, restarting, or resuming this timer.
				// NOTE: The value of this field is preserved while this timer is paused. (Updated on resume)
				std::optional<TimePoint> get_start_point() const
				{
					return start_point;
				}

				// Retrieves the current pause-point. If this timer isn't
				// currently paused, this will return `std::nullopt`.
				std::optional<TimePoint> get_pause_point() const
				{
					return pause_point;
				}

				// Indicates the last known projected end-point.
				// 
				// This is updated when stopping, starting or resuming, but is otherwise preserved.
				// (e.g. the last projected end-point is preserved while paused, but later updated when resuming)
				std::optional<TimePoint> get_projected_end_point() const
				{
					return end_point;
				}

				// Simple alias to default (parameterless) overload of `start`.
				bool activate()
				{
					return start();
				}

				// Alias for `active` method.
				explicit operator bool() const
				{
					return ((active()) && (!completed()));
				}

				// Equivalent to calling `activate`, or `start` with no parameters.
				bool operator()()
				{
					return activate();
				}

				friend auto operator<=>(const TimerImpl&, const TimerImpl&) = default;
			protected:
				std::optional<TimePoint> start_point;
				std::optional<TimePoint> pause_point;
				std::optional<TimePoint> end_point;

				Duration length = {};
		};
	}
}