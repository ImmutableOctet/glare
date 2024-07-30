#include "application.hpp"
#include "window.hpp"

#include "platform/platform_extensions.hpp"

#include <third-party/lib.hpp>

#include <sdl2/SDL_events.h>

#include <utility>

#include <cassert>

// Debugging related:
#include <iostream>
#include <thread>
#include <util/log.hpp>

namespace app
{
	Application::Application(UpdateRate update_rate) :
		update_cadence(update_rate, compute_update_interval(update_rate))
	{
		assert(glare::lib::init_sdl());

		assert(app::platform::enable_high_precision_timers());
	}

	Application::~Application()
	{
		assert(app::platform::disable_high_precision_timers());
	}

	bool Application::start()
	{
		//events.start();

		running = true;

		retrieve_start_time();

		return is_running();
	}

	bool Application::stop()
	{
		//events.stop();
		
		running = false;

		retrieve_stop_time();

		return !is_running();
	}

	Window& Application::make_window(int width, int height, std::string_view title, WindowFlags flags)
	{
		if (window == nullptr)
		{
			window = std::make_unique<app::Window>(width, height, title, flags);
		}

		return (*window);
	}

	void Application::run()
	{
		while (is_running())
		{
			if (!handle_events())
			{
				stop();

				break; // return;
			}

			step_counters.step++;

			auto update_begin_time = TimePoint {};
			auto update_end_time   = TimePoint {};

			auto render_begin_time = TimePoint {};
			auto render_end_time   = TimePoint {};

			const auto prev_step_begin_time   = time_measurements.measurement_time_points.step;
			const auto prev_render_begin_time = time_measurements.measurement_time_points.render;

			const auto step_begin_time = now();

			bool realtime_update_processed = false;
			bool incremental_update_processed = false;

			const bool allow_incremental_update = false; // true;

			if (can_update(now()))
			{
				update_begin_time = now();

				bool allow_realtime_update = true;

				if (allow_incremental_update)
				{
					const auto average_durations = time_measurements.average_durations.get();

					if ((time_measurements.consecutive_times_behind_schedule.update > 0) && (time_measurements.durations.step > update_cadence.interval) && (average_durations.update < update_cadence.interval))
					{
						const auto& fixed_interval = update_cadence.interval;

						//const auto prev_update_time_end = (time_measurements.measurement_time_points.update + time_measurements.durations.update);
						//const auto prev_step_time_end = (prev_update_time_end + (time_measurements.durations.step - time_measurements.durations.update));

						//const auto prev_step_time_end = (time_measurements.measurement_time_points.update + time_measurements.durations.update + fixed_interval);
						
						//const auto prev_step_time_end = (time_measurements.measurement_time_points.step + time_measurements.durations.step - time_measurements.durations.render);
						//const auto prev_step_time_end = (time_measurements.measurement_time_points.update + time_measurements.durations.update);
						//const auto prev_step_time_end = (time_measurements.measurement_time_points.step + time_measurements.durations.step);

						const auto prev_step_time_end = (time_measurements.measurement_time_points.render);

						//const auto& prev_step_time_end = prev_update_time_end;


						auto incremental_frames_processed = TimeStepCounts::Count {};

						auto cumulative_iteration_time = Duration {}; // fixed_interval;
						auto update_time_reclaimed = Duration {};

						//const auto steps_since_last_snapshot   = (durations_per_snapshot.step); // .update // (get_step_counter() - get_step_counter_snapshot());
						//const auto updates_since_last_snapshot = (steps_per_snapshot.update); // (get_update_counter() - get_update_counter_snapshot());

						//const auto update_to_step_ratio = (static_cast<float>(updates_since_last_snapshot) / static_cast<float>(steps_since_last_snapshot));
						//const auto average_update_iteration_time = std::chrono::duration_cast<Duration>(average_durations.update * update_to_step_ratio);

						//const auto update_to_step_ratio = (static_cast<float>(updates_since_last_snapshot) / static_cast<float>(steps_since_last_snapshot));
						//const auto average_update_iteration_time = std::chrono::duration_cast<Duration>(average_durations.update * update_to_step_ratio);

						const auto average_update_iteration_time = compute_update_interval(steps_per_snapshot.update, durations_per_snapshot.update);

						const auto average_render_iteration_time = compute_update_interval(steps_per_snapshot.render, durations_per_snapshot.render);

						//auto average_update_iteration_time = (average_durations.step > average_durations.render)
						//	? (average_durations.step - average_durations.render)
						//	: fixed_interval
						//;

						//const auto smoothed_interval = Duration(static_cast<Duration::rep>(fixed_interval.count() * (static_cast<float>(average_update_iteration_time.count()) / static_cast<float>(fixed_interval.count()))));

						//const auto smoothed_interval = std::chrono::duration_cast<Duration>(fixed_interval * (static_cast<float>(average_update_iteration_time.count()) / static_cast<float>(fixed_interval.count())));

						const auto smoothed_interval = std::min(average_update_iteration_time, fixed_interval); // fixed_interval;

						const auto projected_update_end_time = (update_begin_time + fixed_interval);

						while (update_time_reclaimed < time_measurements.cumulative_time_behind_schedule.update)
						{
							const auto incremental_update_begin = now();

							if (incremental_update_begin >= projected_update_end_time)
							{
								break;
							}
							
							const auto incremental_update_time = (prev_step_time_end + cumulative_iteration_time);

							if (incremental_update_time >= projected_update_end_time)
							{
								break;
							}

							if (!can_update(incremental_update_time))
							{
								break;
							}

							update(incremental_update_time, true); step_counters.update++;

							const auto incremental_update_end = now();

							auto iteration_time = (incremental_update_end - incremental_update_begin);

							if (iteration_time < smoothed_interval)
							{
								sleep((smoothed_interval - iteration_time), true, true, false);

								const auto adjusted_incremental_update_end = now();

								iteration_time = (adjusted_incremental_update_end - incremental_update_begin);
							}

							cumulative_iteration_time += iteration_time;

							update_time_reclaimed += fixed_interval;

							//print("incremental_frame: #{} (Real: {}ms) (Fixed to: {}ms)", incremental_frames_processed, std::chrono::duration_cast<std::chrono::milliseconds>(incremental_update_end - incremental_update_begin).count(), std::chrono::duration_cast<std::chrono::milliseconds>(iteration_time).count());

							incremental_frames_processed++;
						}

						incremental_update_processed = (incremental_frames_processed > 0);

						//print("incremental_frames_processed: {}", incremental_frames_processed);

						if (incremental_update_processed)
						{
							if (update_time_reclaimed >= time_measurements.cumulative_time_behind_schedule.update)
							{
								// TODO: Look into removing properly*
								time_measurements.consecutive_times_behind_schedule.update = 0;
								time_measurements.cumulative_time_behind_schedule.update   = {};
							}
							else
							{
								time_measurements.cumulative_time_behind_schedule.update -= update_time_reclaimed;
							}
						}

						allow_realtime_update = (now() < projected_update_end_time); // update_begin_time
					}
				}

				const auto realtime_update_timestamp = (incremental_update_processed)
					? now()
					: update_begin_time
				;

				if (allow_realtime_update)
				{
					if ((!allow_incremental_update) || (incremental_update_processed) || can_update(realtime_update_timestamp))
					{
						update(realtime_update_timestamp, false); step_counters.update++;
					}

					realtime_update_processed = true;
				}

				update_end_time = now();
			}

			const auto pending_render_begin_time = now();

			const bool render_processed = true; // can_render(pending_render_begin_time);

			if (render_processed)
			{
				render_begin_time = now(); // pending_render_begin_time;

				begin_render(render_begin_time);
			
				render(render_begin_time); step_counters.render++;

				end_render(render_begin_time);

				//const auto sleep_time = 20; // (std::rand() % 50);

				//print("Sleeping for {}...", sleep_time);

				//sleep(std::chrono::milliseconds { sleep_time });

				render_end_time = now();
			}

			const auto step_end_time = now();

			auto& measurements_out = time_measurements;

			const auto step_duration   = (  step_end_time - step_begin_time  );
			const auto update_duration = (update_end_time - update_begin_time);
			const auto render_duration = (render_end_time - render_begin_time);

			const bool render_skipped = (!render_processed);
			const bool update_skipped = ((!realtime_update_processed) || ((incremental_update_processed) && (measurements_out.consecutive_times_behind_schedule.update > 0)));

			update_time_measurements
			(
				TimeStepPoints
				{
					.step   = (step_begin_time),
					.update = ((update_skipped) ? measurements_out.measurement_time_points.update : update_begin_time),
					.render = ((render_skipped) ? measurements_out.measurement_time_points.render : render_begin_time)
				},

				TimeStepDurations
				{
					.step   = (step_duration),
					.update = ((update_skipped) ? measurements_out.durations.update : update_duration),
					.render = ((render_skipped) ? measurements_out.durations.render : render_duration)
				}
			);

			update_lag_indicators(measurements_out, update_skipped, render_skipped);

			update_steps_per_second(now());

			const auto has_prev_wait_compensation_time = has_wait_compensation_time();
			const auto prev_wait_compensation_time = get_wait_compensation_time();

			wait_compensation_time = wait(step_begin_time);

			if (has_prev_wait_compensation_time)
			{
				auto compensation_time = prev_wait_compensation_time;

				const auto prev_render_begin_time_offset = (prev_render_begin_time - prev_step_begin_time);

				const auto current_render_begin_time_offset = (pending_render_begin_time - step_begin_time); // render_begin_time

				if (current_render_begin_time_offset > prev_render_begin_time_offset)
				{
					const auto relative_render_begin_time_delta = (current_render_begin_time_offset - prev_render_begin_time_offset);

					if (relative_render_begin_time_delta <= compensation_time)
					{
						compensation_time -= relative_render_begin_time_delta;
					}
				}

				sleep(compensation_time, true, false, false);
			}
		}
	}

	bool Application::handle_events()
	{
		SDL_Event e;
		
		//int events_processed = 0;

		while (SDL_PollEvent(&e)) // SDL_WaitEvent
		{
			if (process_event(e))
			{
				//events_processed++;

				continue;
			}

			switch (e.type)
			{
				case SDL_QUIT:
					return false;

				case SDL_WINDOWEVENT:
				{
					const auto& window_event = e.window;

					if (window->get_id() == window_event.windowID)
					{
						if (!window->process_event(e, window_event, *this))
						{
							return false;
						}
					}

					break;
				}

				case SDL_KEYDOWN:
					on_keydown(e.key);

					break;

				case SDL_KEYUP:
					on_keyup(e.key);

					break;

				default:
					continue;
			}

			//events_processed++;
		}

		//return events_processed;
		return true;
	}

	bool Application::can_update(TimePoint time) const
	{
		if ((time_measurements.consecutive_times_behind_schedule.update == 0) && (time_measurements.consecutive_times_behind_schedule.render == 0))
		{
			const auto updates_since_last_snapshot = (get_update_counter() - get_update_counter_snapshot());

			if (updates_since_last_snapshot >= get_update_rate())
			{
				return false;
			}
		}

		return true;
	}

	bool Application::can_render(TimePoint time) const
	{
		using namespace std::literals::chrono_literals;

		const auto average_durations = time_measurements.average_durations.get();

		const auto& target_render_interval = update_cadence.interval;

		const auto expected_render_time = (average_durations)
			? average_durations.render
			: update_cadence.interval
		;

		const auto time_since_last_render = (time - time_measurements.measurement_time_points.render);

		const auto max_time_between_renders = std::min((expected_render_time * 3), (target_render_interval * 3)); // std::chrono::duration_cast<Duration>(500ms); // 250ms;

		if (time_since_last_render >= max_time_between_renders)
		{
			return true;
		}

		// Compensate for slow update times:
		const auto update_compensation_threshold = 10;

		if (time_measurements.consecutive_times_behind_schedule.update >= update_compensation_threshold)
		{
			if (time_measurements.consecutive_times_improving.update < update_compensation_threshold)
			{
				if (time_measurements.durations.render < time_measurements.durations.update)
				{
					const auto& target_update_interval = update_cadence.interval;

					const auto expected_update_time = (average_durations)
						? average_durations.update
						: update_cadence.interval
					;

					const auto fast_render_time_threshold = (target_render_interval / 2);
					const auto render_significance_threshold = 0.10f; // At least X% of time is spent rendering.
					const auto update_catchup_threshold = 0.5f;

					const auto render_percentage_of_budget = (static_cast<float>(time_measurements.durations.render.count()) / static_cast<float>(time_measurements.durations.step.count()));
					const auto update_projected_percentage_of_budget = (static_cast<float>(expected_update_time.count()) / static_cast<float>(target_update_interval.count()));

					if
					(
						(
							(time_measurements.consecutive_times_behind_schedule.render < (time_measurements.consecutive_times_behind_schedule.update / 2))
							||
							(time_measurements.durations.render < fast_render_time_threshold)
							||
							(update_projected_percentage_of_budget < update_catchup_threshold)
						)
						&&
						(render_percentage_of_budget > render_significance_threshold)
					)
					{
						const auto expected_render_time = (average_durations)
							? average_durations.render
							: update_cadence.interval
						;

						if (time_measurements.cumulative_time_behind_schedule.update > expected_render_time)
						{
							return false;
						}
					}
				}
			}
		}

		// Compensate for slow render times:
		const auto render_compensation_threshold = 30;

		if (time_measurements.consecutive_times_behind_schedule.render >= render_compensation_threshold)
		{
			if (time_measurements.consecutive_times_improving.render < render_compensation_threshold)
			{
				if (time_measurements.durations.update < time_measurements.durations.render)
				{
					if (time_measurements.cumulative_time_behind_schedule.render > target_render_interval)
					{
						const auto& target_update_interval = update_cadence.interval;

						const auto expected_step_time = (average_durations)
							? average_durations.step
							: update_cadence.interval
						;

						const auto render_significance_threshold = 0.60f; // At least X% of time is spent rendering.

						// Alternative implementation:
						//const auto render_percentage_of_budget = (static_cast<float>(expected_render_time.count()) / static_cast<float>(expected_step_time.count()));

						const auto render_percentage_of_budget = (static_cast<float>(time_measurements.durations.render.count()) / static_cast<float>(expected_step_time.count()));

						if (render_percentage_of_budget >= render_significance_threshold)
						{
							const auto expected_update_time = (average_durations)
								? average_durations.update
								: update_cadence.interval
							;

							const auto update_projected_percentage_of_budget = (static_cast<float>(expected_update_time.count()) / static_cast<float>(target_update_interval.count())); // expected_step_time.count()

							if (update_projected_percentage_of_budget < render_percentage_of_budget)
							{
								return false;
							}
						}
					}
				}
			}
		}

		return true;
	}

	void Application::execute()
	{
		std::cout << "Executing start-up procedure...\n";

		start();

		on_execute();

		std::cout << "Running...\n";

		run();

		std::cout << "Shutting down...\n";

		stop();

		on_shutdown();

		std::cout << "Done.\n";
	}

	void Application::begin_render(TimePoint time) {}
	void Application::render(TimePoint time) {}
	void Application::end_render(TimePoint time) {}

	Application::Duration Application::wait(TimePoint step_begin_time, bool exhaust_time_remainder)
	{
		return {};
	}

	void Application::on_step_counter_snapshot(TimeStepCounts counts_per_snapshot) {}
	
	entt::dispatcher* Application::get_event_handler() { return nullptr; }

	void Application::on_keydown(const keyboard_event_t& event) {}
	void Application::on_keyup(const keyboard_event_t& event) {}
	void Application::on_window_resize(Window& window, int width, int height) {}

	void Application::sleep(Duration duration, bool allow_spin_lock, bool allow_short_sleep, bool sleep_on_one_ms)
	{
		using SleepClockType = std::chrono::high_resolution_clock; // std::chrono::steady_clock; // std::chrono::high_resolution_clock;

		const auto duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();

		// Enforce millisecond precision for sleep intervals on Windows.
		app::platform::sleep<SleepClockType>(duration, duration_ms, allow_spin_lock, allow_short_sleep, sleep_on_one_ms);
	}

	void Application::sleep(std::chrono::milliseconds duration)
	{
		sleep(std::chrono::duration_cast<Duration>(duration));
	}

	void Application::sleep(Milliseconds duration)
	{
		sleep(std::chrono::milliseconds { duration });
	}

	bool Application::has_wait_compensation_time() const
	{
		return (wait_compensation_time != Duration {});
	}

	Application::Duration Application::get_wait_compensation_time() const
	{
		return wait_compensation_time;
	}

	bool Application::process_event(const SDL_Event& e)
	{
		if (input.process_event(e, get_event_handler()))
		{
			return true;
		}

		return false;
	}

	Application::TimePoint Application::now() const
	{
		return SystemClock::now();
	}

	Application::Duration Application::time() const
	{
		return now().time_since_epoch();
	}

	Milliseconds Application::unix_time() const
	{
		return static_cast<Milliseconds>(std::chrono::duration_cast<std::chrono::milliseconds>(time()).count());
	}

	Milliseconds Application::milliseconds() const
	{
		return static_cast<Milliseconds>(std::chrono::duration_cast<std::chrono::milliseconds>(now() - start_point).count());
	}

	void Application::on_execute() {}

	void Application::on_shutdown() {}

	void Application::retrieve_start_time()
	{
		start_point = now();
	}

	void Application::retrieve_stop_time()
	{
		stop_point = now();
	}

	void Application::update_time_measurements(const TimeStepPoints& measurement_time_points, const TimeStepDurations& measured_durations)
	{
		const auto expected_strides = TimeStepDurations { update_cadence.interval, update_cadence.interval, update_cadence.interval };

		const auto measured_strides = (this->time_measurements.measurement_time_points)
			? (measurement_time_points - this->time_measurements.measurement_time_points)
			: TimeStepDurations {}
		;

		const auto previous_durations = this->time_measurements.durations;

		const auto duration_difference_from_previous = (measured_durations - previous_durations);
		const auto duration_percentage_change_from_previous = -(duration_difference_from_previous / previous_durations);
		
		const auto previous_strides = this->time_measurements.strides;

		const auto stride_difference_from_previous = (measured_strides - previous_strides);
		const auto stride_percentage_change_from_previous = (stride_difference_from_previous / previous_strides);

		const auto stride_difference_from_expectation = (measured_strides - expected_strides);
		const auto stride_percentage_behind_expectation = (stride_difference_from_expectation / expected_strides);

		auto& measurements_out = this->time_measurements;

		measurements_out.measurement_time_points            = measurement_time_points;
		measurements_out.durations                          = measured_durations;
		measurements_out.strides                            = measured_strides;
		measurements_out.stride_difference_from_expectation = stride_difference_from_expectation;
		measurements_out.duration_percentage_improvement    = duration_percentage_change_from_previous;
		measurements_out.stride_percentage_change           = stride_percentage_change_from_previous;
		measurements_out.stride_percentage_behind           = stride_percentage_behind_expectation;

		measurements_out.average_durations.sample(measurements_out.durations);

		measurements_out.snapshot_durations = (measurements_out.snapshot_durations + measured_durations);
	}

	void Application::update_lag_indicators(TimeMeasurements& time_measurements, bool update_skipped, bool render_skipped)
	{
		constexpr auto lag_threshold = 0.06f; // % of extra time spent
		constexpr auto improvement_threshold = lag_threshold;

		const bool step_skipped = false;

		if (step_skipped)
		{
			++time_measurements.consecutive_times_skipped.step;
		}
		else
		{
			time_measurements.consecutive_times_skipped.step = {};

			if (time_measurements.stride_percentage_behind.step >= lag_threshold)
			{
				++time_measurements.consecutive_times_behind_schedule.step;

				time_measurements.cumulative_time_behind_schedule.step += time_measurements.stride_difference_from_expectation.step;
			}
			else
			{
				time_measurements.consecutive_times_behind_schedule.step = {};
				time_measurements.cumulative_time_behind_schedule.step = {};
			}

			if (time_measurements.stride_difference_from_expectation.step.count() < 0)
			{
				++time_measurements.consecutive_times_ahead_of_schedule.step;
			}
			else
			{
				time_measurements.consecutive_times_ahead_of_schedule.step = {};
			}

			if (time_measurements.duration_percentage_improvement.step >= improvement_threshold)
			{
				++time_measurements.consecutive_times_improving.step;
			}
			else
			{
				time_measurements.consecutive_times_improving.step = {};
			}
		}

		if (update_skipped)
		{
			++time_measurements.consecutive_times_skipped.update;
		}
		else
		{
			time_measurements.consecutive_times_skipped.update = {};

			if (time_measurements.stride_percentage_behind.update >= lag_threshold)
			{
				++time_measurements.consecutive_times_behind_schedule.update;

				time_measurements.cumulative_time_behind_schedule.update += time_measurements.stride_difference_from_expectation.update;
			}
			else
			{
				time_measurements.consecutive_times_behind_schedule.update = {};
				time_measurements.cumulative_time_behind_schedule.update = {};
			}

			if (time_measurements.stride_difference_from_expectation.update.count() < 0)
			{
				++time_measurements.consecutive_times_ahead_of_schedule.update;
			}
			else
			{
				time_measurements.consecutive_times_ahead_of_schedule.update = {};
			}

			if (time_measurements.duration_percentage_improvement.update >= improvement_threshold)
			{
				++time_measurements.consecutive_times_improving.update;
			}
			else
			{
				time_measurements.consecutive_times_improving.update = {};
			}
		}

		if (render_skipped)
		{
			++time_measurements.consecutive_times_skipped.render;
		}
		else
		{
			time_measurements.consecutive_times_skipped.render = {};

			if (time_measurements.stride_percentage_behind.render >= lag_threshold)
			{
				++time_measurements.consecutive_times_behind_schedule.render;

				time_measurements.cumulative_time_behind_schedule.render += time_measurements.stride_difference_from_expectation.render;
			}
			else
			{
				time_measurements.consecutive_times_behind_schedule.render = {};
				time_measurements.cumulative_time_behind_schedule.render = {};
			}

			if (time_measurements.stride_difference_from_expectation.render.count() < 0)
			{
				++time_measurements.consecutive_times_ahead_of_schedule.render;
			}
			else
			{
				time_measurements.consecutive_times_ahead_of_schedule.render = {};
			}

			if (time_measurements.duration_percentage_improvement.render >= improvement_threshold)
			{
				++time_measurements.consecutive_times_improving.render;
			}
			else
			{
				time_measurements.consecutive_times_improving.render = {};
			}
		}
	}

	void Application::update_steps_per_second(TimePoint time)
	{
		using namespace std::literals::chrono_literals;

		if (const auto time_elapsed = (time - last_step_counter_snapshot_timer); time_elapsed >= 1s)
		{
			last_step_counter_snapshot_timer = time;

			const auto step_counter_delta = (step_counters - last_step_counter_snapshot);

			last_step_counter_snapshot = step_counters;

			steps_per_snapshot = step_counter_delta;

			//const auto duration_deltas = (snapshot_durations - durations_per_snapshot);

			durations_per_snapshot = time_measurements.snapshot_durations;

			time_measurements.snapshot_durations = {};

			on_step_counter_snapshot(steps_per_snapshot);
		}
	}
}