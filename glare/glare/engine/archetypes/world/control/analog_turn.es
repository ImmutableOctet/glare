realtime
	repeat
		auto analog_input = yield(OnRelativeAnalogInput::analog == Analog::Movement)

		auto rate = (0.5 * delta)

		player_model.set_yaw(analog_input.angle, rate)
	end
end