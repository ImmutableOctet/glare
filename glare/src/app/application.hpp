#pragma once

#include "types.hpp"
#include "window_flags.hpp"
#include "input/input.hpp"

#include <entt/signal/dispatcher.hpp>

#include <string_view>
#include <atomic>
#include <chrono>
#include <cstdint>

// SDL:
struct SDL_KeyboardEvent;
union SDL_Event;

namespace app
{
	class Window;

	class Application
	{
		public:
			using SystemClock = std::chrono::steady_clock; // std::chrono::system_clock;
			using TimePoint   = SystemClock::time_point;
			using Duration    = SystemClock::duration;
			using Interval    = Duration;

			using Counter     = StepCounter;

			using UpdateRate = app::UpdateRate;

			using keyboard_event_t = SDL_KeyboardEvent;

			struct UpdateCadence
			{
				UpdateRate rate     = {};
				Interval   interval = {};
			};

			struct TimeStepDeltas
			{
				using Scalar = float;

				Scalar step   = static_cast<Scalar>(1.0);
				Scalar update = static_cast<Scalar>(1.0);
				Scalar render = static_cast<Scalar>(1.0);

				constexpr TimeStepDeltas operator/(const TimeStepDeltas& comparison_deltas) const
				{
					return TimeStepDeltas
					{
						.step   = (this->step   / comparison_deltas.step),
						.update = (this->update / comparison_deltas.update),
						.render = (this->render / comparison_deltas.render)
					};
				}

				constexpr TimeStepDeltas operator-() const
				{
					return TimeStepDeltas
					{
						.step   = (-this->step),
						.update = (-this->update),
						.render = (-this->render)
					};
				}

				constexpr TimeStepDeltas operator~() const
				{
					return TimeStepDeltas
					{
						.step   = (static_cast<Scalar>(1.0) - this->step),
						.update = (static_cast<Scalar>(1.0) - this->update),
						.render = (static_cast<Scalar>(1.0) - this->render)
					};
				}
			};

			struct TimeStepDurations
			{
				Duration step   = {};
				Duration update = {};
				Duration render = {};

				constexpr TimeStepDurations operator+(const TimeStepDurations& durations) const
				{
					return TimeStepDurations
					{
						.step   = (this->step   + durations.step),
						.update = (this->update + durations.update),
						.render = (this->render + durations.render)
					};
				}

				constexpr TimeStepDurations operator-(const TimeStepDurations& durations) const
				{
					return TimeStepDurations
					{
						.step   = (this->step   - durations.step),
						.update = (this->update - durations.update),
						.render = (this->render - durations.render)
					};
				}

				constexpr TimeStepDeltas operator/(const TimeStepDurations& expected_durations) const
				{
					using Scalar = TimeStepDeltas::Scalar;

					return TimeStepDeltas
					{
						.step   = (static_cast<Scalar>(this->step.count())   / static_cast<Scalar>(expected_durations.step.count())),
						.update = (static_cast<Scalar>(this->update.count()) / static_cast<Scalar>(expected_durations.update.count())),
						.render = (static_cast<Scalar>(this->render.count()) / static_cast<Scalar>(expected_durations.render.count()))
					};
				}

				constexpr TimeStepDurations operator/(Duration::rep denominator) const
				{
					return TimeStepDurations
					{
						.step   = (this->step   / denominator),
						.update = (this->update / denominator),
						.render = (this->render / denominator)
					};
				}

				constexpr explicit operator bool() const
				{
					constexpr auto zero_duration = Duration {}; // Duration::zero();

					return
					(
						(step   != zero_duration)
						&&
						(update != zero_duration)
						&&
						(render != zero_duration)
					);
				}
			};

			struct TimeStepDurationAverages
			{
				using IndexType = std::uint8_t;

				static constexpr std::size_t duration_sample_size = 60; // 20;

				std::array<TimeStepDurations, duration_sample_size> durations_sampled;

				IndexType index = {};
				
				constexpr void sample(const TimeStepDurations& durations)
				{
					durations_sampled[index] = durations;

					index = static_cast<IndexType>((static_cast<std::size_t>(index) + static_cast<std::size_t>(1)) % durations_sampled.size());
				}

				constexpr TimeStepDurations get() const
				{
					auto durations = TimeStepDurations {};

					auto durations_found = Duration::rep {};

					for (const auto& sample_entry : durations_sampled)
					{
						if (sample_entry)
						{
							durations = (durations + sample_entry);

							++durations_found;
						}
					}

					if (durations_found > 0)
					{
						durations = (durations / durations_found);
					}

					return durations;
				}
			};

			struct TimeStepPoints
			{
				TimePoint step   = {};
				TimePoint update = {};
				TimePoint render = {};

				constexpr TimeStepDurations operator-(const TimeStepPoints& time_points) const
				{
					return TimeStepDurations
					{
						.step   = (this->step   - time_points.step),
						.update = (this->update - time_points.update),
						.render = (this->render - time_points.render)
					};
				}

				constexpr explicit operator bool() const
				{
					constexpr auto zero_point = TimePoint {};

					return
					(
						(step   != zero_point)
						&&
						(update != zero_point)
						&&
						(render != zero_point)
					);
				}
			};

			struct TimeStepCounts
			{
				using Count = app::StepCounter;

				Count step   = {};
				Count update = {};
				Count render = {};

				constexpr TimeStepCounts operator+(const TimeStepCounts& counters) const
				{
					return TimeStepCounts
					{
						.step   = (this->step   + counters.step),
						.update = (this->update + counters.update),
						.render = (this->render + counters.render)
					};
				}

				constexpr TimeStepCounts operator-(const TimeStepCounts& counters) const
				{
					return TimeStepCounts
					{
						.step   = (this->step   - counters.step),
						.update = (this->update - counters.update),
						.render = (this->render - counters.render)
					};
				}

				constexpr TimeStepCounts operator/(const TimeStepCounts& counters) const
				{
					return TimeStepCounts
					{
						.step   = (this->step   / counters.step),
						.update = (this->update / counters.update),
						.render = (this->render / counters.render)
					};
				}

				constexpr explicit operator bool() const
				{
					constexpr auto zero = Counter {};

					return
					(
						(step   != zero)
						&&
						(update != zero)
						&&
						(render != zero)
					);
				}
			};

			struct TimeMeasurements
			{
				using Count = TimeStepCounts::Count;

				TimeStepPoints           measurement_time_points;

				TimeStepDurationAverages average_durations;

				TimeStepDurations        durations;
				TimeStepDurations        strides;
				TimeStepDurations        stride_difference_from_expectation;
				TimeStepDurations        cumulative_time_behind_schedule;
				TimeStepDurations        snapshot_durations;

				TimeStepDeltas           duration_percentage_improvement;
				TimeStepDeltas           stride_percentage_change;
				TimeStepDeltas           stride_percentage_behind;

				TimeStepCounts           consecutive_times_behind_schedule;
				TimeStepCounts           consecutive_times_ahead_of_schedule;
				TimeStepCounts           consecutive_times_improving;

				TimeStepCounts           consecutive_times_skipped;
			};

		protected:
			friend class Window;
			
			Application(UpdateRate update_rate);

			virtual ~Application();

			Window& make_window(int width, int height, std::string_view title="", WindowFlags flags=WindowFlags::Default);

			TimePoint now() const;
			Duration time() const;
			
			Milliseconds unix_time() const;
			Milliseconds milliseconds() const;

			virtual void on_execute();
			virtual void on_shutdown();

			virtual bool handle_events();

			virtual bool can_update(TimePoint time) const;
			virtual bool can_render(TimePoint time) const;

			virtual void update(TimePoint time, bool incremental) = 0;

			virtual void render(TimePoint time);
			virtual void begin_render(TimePoint time);
			virtual void end_render(TimePoint time);

			virtual Duration wait(TimePoint step_begin_time, bool exhaust_time_remainder=false);

			virtual void on_step_counter_snapshot(TimeStepCounts counts_per_snapshot);

			// Optional; empty implementation.
			virtual entt::dispatcher* get_event_handler();

			virtual void on_keydown(const keyboard_event_t& event);
			virtual void on_keyup(const keyboard_event_t& event);
			virtual void on_window_resize(Window& window, int width, int height);

			// Returning `true` indicates that the event was handled, and will therefore
			// not need to be processed further by the default implementation of `handle_events`.
			virtual bool process_event(const SDL_Event& e);

			// Starts all additional threads associated with the application.
			bool start();

			// Executes the main application thread.
			void run();

			// Stops all additional threads associated with the application.
			// To ensure proper finalization, this should be executed after calling 'run'.
			bool stop();

			void sleep(Duration duration, bool allow_spin_lock=false, bool allow_short_sleep=false, bool sleep_on_one_ms=false);
			void sleep(std::chrono::milliseconds duration);
			void sleep(Milliseconds duration);

			bool has_wait_compensation_time() const;
			Duration get_wait_compensation_time() const;

		protected:
			static constexpr Interval compute_update_interval(auto update_rate, auto time_period)
			{
				return std::chrono::duration_cast<Interval>
				(
					std::chrono::duration_cast<std::chrono::nanoseconds>(time_period)
					/
					update_rate
				);
			}

			static constexpr Interval compute_update_interval(auto update_rate)
			{
				return compute_update_interval(update_rate, std::chrono::seconds { 1 });
			}

			std::unique_ptr<Window> window;

			TimePoint               start_point = {};
			TimePoint               stop_point  = {};

			Duration                wait_compensation_time = {};

			// TODO: Migrate this field to the `Game` class.
			input::InputHandler     input;

		private:
			TimeMeasurements        time_measurements;

			TimeStepCounts          step_counters;
			TimeStepCounts          steps_per_snapshot;
			TimeStepDurations       durations_per_snapshot;
			TimeStepCounts          last_step_counter_snapshot;
			TimePoint               last_step_counter_snapshot_timer = {};

			UpdateCadence           update_cadence;

			void retrieve_start_time();
			void retrieve_stop_time();

			void update_time_measurements(const TimeStepPoints& measurement_time_points, const TimeStepDurations& measured_durations);
			void update_lag_indicators(TimeMeasurements& time_measurements, bool update_skipped, bool render_skipped);
			void update_steps_per_second(TimePoint time);

		protected:
			bool running : 1 = false;

		public:
			void execute();

			//inline bool is_render_out_of_sync() const { return (render_counter != update_counter); }

			inline bool is_running() const { return running; }
			
			inline Window* get_window() const { return window.get(); }

			inline UpdateRate get_update_rate() const
			{
				return update_cadence.rate;
			}

			inline Interval get_update_interval() const
			{
				return update_cadence.interval;
			}

			inline Counter get_step_counter() const
			{
				return step_counters.step;
			}

			inline Counter get_update_counter() const
			{
				return step_counters.update;
			}

			inline Counter get_render_counter() const
			{
				return step_counters.render;
			}

			inline Counter get_step_counter_snapshot() const
			{
				return last_step_counter_snapshot.step;
			}

			inline Counter get_update_counter_snapshot() const
			{
				return last_step_counter_snapshot.update;
			}

			inline Counter get_render_counter_snapshot() const
			{
				return last_step_counter_snapshot.render;
			}

			inline Duration get_last_step_duration() const
			{
				return time_measurements.durations.step;
			}

			inline Duration get_last_update_duration() const
			{
				return time_measurements.durations.update;
			}

			inline Duration get_last_render_duration() const
			{
				return time_measurements.durations.render;
			}

			inline const TimeMeasurements& get_time_measurements() const
			{
				return time_measurements;
			}

			inline TimePoint get_start_point() const
			{
				return start_point;
			}

			inline TimePoint get_stop_point() const
			{
				return stop_point;
			}
	};
}