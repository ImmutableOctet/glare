realtime
	repeat
		auto analog_input = yield(OnRelativeAnalogInput::analog == Analog::Movement)

		auto rate = (length(analog_input.value) * delta)

		MotionSystem::accelerate(self, analog_input.direction, rate)
	end
end