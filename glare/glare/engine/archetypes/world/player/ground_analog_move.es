realtime
	repeat
		auto analog_input = yield(OnRelativeAnalogInput::analog == Analog::Movement)

		//print("Analog direction:")
		//print(analog_input.direction)
		
		//self.DirectionalComponent = DirectionalComponent(analog_input.direction)

		//self.direction_vector = analog_input.direction
	end
end