#include "graphics_application.hpp"

#include <imgui_impl_sdl.h>

#include <chrono>
#include <thread>

namespace app
{
	Graphics::Graphics(app::Window& window, WindowFlags flags, bool vsync, bool extensions)
		: extensions(extensions)
	{
		if (!(flags & WindowFlags::OpenGL))
		{
			throw std::runtime_error("OpenGL is currently the only supported graphics-backend.");
		}

		auto graphics_flags = graphics::ContextFlags::Default;

		if (vsync)
		{
			graphics_flags |= graphics::ContextFlags::VSync;
		}
		else
		{
			graphics_flags &= ~graphics::ContextFlags::VSync;
		}

		context = std::make_unique<graphics::Context>(window, graphics::Backend::OpenGL, graphics_flags, extensions);

		// Create the default canvas.
		canvas = std::make_unique<graphics::Canvas>(context);

		//max_framerate = 100;
		//max_framerate = 60;
		max_framerate = 300;
	}

	bool GraphicsApplication::vsync_enabled() const
	{
		if (graphics.context)
		{
			return (graphics.context->get_flag(graphics::ContextFlags::VSync));
		}

		return false;
	}

	void GraphicsApplication::begin_render(TimePoint time)
	{
		graphics.canvas->begin();
	}

	void GraphicsApplication::end_render(TimePoint time)
	{
		graphics.canvas->end();

		graphics.canvas->flip(*window);
	}

	bool GraphicsApplication::process_event(const SDL_Event& e)
	{
		if (imgui_enabled())
		{
			const auto& mouse = input.get_mouse();
			
			if (!mouse.locked())
			{
				if (ImGui_ImplSDL2_ProcessEvent(&e))
				{
					if (e.type != SDL_WINDOWEVENT)
					{
						return true;
					}
				}
			}
		}

		return Application::process_event(e);
	}

	bool GraphicsApplication::can_update(TimePoint time) const
	{
		if (vsync_enabled())
		{
			return true;
		}

		if (graphics.max_framerate == Graphics::MAX_FRAMERATE_UNCAPPED)
		{
			return true;
		}

		const auto& time_measurements = get_time_measurements();

		const bool updates_on_schedule = (time_measurements.consecutive_times_behind_schedule.update == 0);

		const bool renders_on_schedule = (time_measurements.consecutive_times_behind_schedule.render == 0);

		if ((updates_on_schedule) && (renders_on_schedule))
		{
			const auto updates_since_last_snapshot = (get_update_counter() - get_update_counter_snapshot());

			if (updates_since_last_snapshot >= graphics.max_framerate)
			{
				return false;
			}
		}
		//else if (!renders_on_schedule)
		//{
		//	const auto updates_since_last_snapshot = (get_update_counter() - get_update_counter_snapshot());
		//
		//	if (updates_since_last_snapshot >= get_update_rate())
		//	{
		//		//print("Too many frames: {}ms to update on average", std::chrono::duration_cast<std::chrono::milliseconds>(time_measurements.average_durations.get().update).count());
		//
		//		return false;
		//	}
		//}

		return true;
	}

	bool GraphicsApplication::can_render(TimePoint time) const
	{
		return true;

		const auto target_framerate = get_update_rate();

		if (graphics.framerate >= target_framerate)
		{
			if (graphics.framerate >= graphics.max_framerate)
			{
				return false;
			}

			/*
			// Disabled for now:
			const auto& time_measurements = get_time_measurements();

			if (time_measurements.consecutive_times_skipped.render >= (target_framerate / 4)) // (graphics.average_framerate / 4)
			{
				return true;
			}
			*/
		}

		return Application::can_render(time);
	}

	GraphicsApplication::Duration GraphicsApplication::wait(TimePoint step_begin_time, bool exhaust_time_remainder)
	{
		const bool vsync = vsync_enabled();

		if (vsync)
		{
			return {};
		}

		if (graphics.max_framerate == Graphics::MAX_FRAMERATE_UNCAPPED)
		{
			return {};
		}

		const auto& time_measurements = get_time_measurements();

		const auto step_interval = compute_update_interval(graphics.max_framerate); // get_update_interval();

		using namespace std::literals::chrono_literals;

#if _WIN32
		// Windows doesn't allow sleeping for less than 1ms.
		//if (step_interval >= 1ms)
#endif // _WIN32
		{
			const auto step_intended_end_time = (step_begin_time + step_interval);

			if (now() < step_intended_end_time) // (step_time_elapsed < step_interval)
			{
				//const auto step_time_elapsed = time_measurements.durations.step;

				const auto step_time_remaining = (step_intended_end_time - now()); // (step_interval - step_time_elapsed);

				const auto fixed_interval = get_update_interval();

				//print("step_interval: {}us ({}ms) -- Fixed: {}us ({}ms)", std::chrono::duration_cast<std::chrono::microseconds>(step_interval).count(), std::chrono::duration_cast<std::chrono::milliseconds>(step_interval).count(), std::chrono::duration_cast<std::chrono::microseconds>(fixed_interval).count(), std::chrono::duration_cast<std::chrono::milliseconds>(fixed_interval).count());
				//print("{} elapsed ({}ms), {}us remaining ({}ms)", std::chrono::duration_cast<std::chrono::microseconds>(step_time_elapsed).count(), std::chrono::duration_cast<std::chrono::milliseconds>(step_time_elapsed).count(), std::chrono::duration_cast<std::chrono::microseconds>(step_time_remaining).count(), std::chrono::duration_cast<std::chrono::milliseconds>(step_time_remaining).count());

				//print("======== Waiting for next frame: {}ms ========", std::chrono::duration_cast<std::chrono::milliseconds>(step_time_remaining).count());

				//auto sleep_begin_time = now();

				sleep(step_time_remaining, exhaust_time_remainder, true, true);
				///sleep(step_time_remaining, true, true);

				//sleep(std::chrono::milliseconds(10));

				//std::this_thread::sleep_until(time_measurements.measurement_time_points.step + step_interval);

				auto sleep_end_time = now();

				//const auto time_sleeping = (sleep_end_time - sleep_begin_time);

				//print("Slept for {}us ({}ms)", std::chrono::duration_cast<std::chrono::microseconds>(time_sleeping).count(), std::chrono::duration_cast<std::chrono::milliseconds>(time_sleeping).count());
				//print("Frame time: {}us", std::chrono::duration_cast<std::chrono::microseconds>(end_time - time_measurements.measurement_time_points.step).count());

				if (sleep_end_time < step_intended_end_time)
				{
					const auto time_remainder = (step_intended_end_time - sleep_end_time); // (step_time_remaining - time_sleeping);

					return time_remainder;
				}
			}
		}

		return {};
	}

	void GraphicsApplication::on_step_counter_snapshot(TimeStepCounts counts_per_snapshot)
	{
		graphics.framerate = static_cast<UpdateRate>(counts_per_snapshot.render);
		//graphics.framerate = static_cast<UpdateRate>(counts_per_snapshot.update);
	}
}