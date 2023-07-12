realtime
	repeat
		auto analog_input = yield(OnRelativeAnalogInput::analog == Analog::Movement)

		auto turning_speed = (0.5 * delta)

		MotionSystem::influence_motion_direction(self, analog_input.direction, turning_speed)
	end
end