thread(player_jump_add_velocity)
	repeat
		print("Adding...")
		
		self.velocity.velocity += Vector(0.0, (0.2 * delta), 0.0)
	end
end


print("Waiting four seconds...")

sleep(4.0)

print("Stopping thread.")

player_jump_add_velocity.stop()