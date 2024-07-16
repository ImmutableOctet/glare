auto ray = CameraSystem::get_ray_from_display_coordinates(self.Transform2DComponent::position)

auto result = PhysicsSystem::directional_ray_cast(ray.origin, ray.direction)

if (result)
	auto hit_entity = result.hit_entity

	//print("has_comp:")
	//print(has_comp)
	
	if (InstanceComponent::has_component(hit_entity))
		result.hit_entity.EditorPropertiesComponent::is_selected = true // hit_entity.
		
		print("Hit entity:")
		print(result.hit_entity)

		print("is_selected:")
		print(result.hit_entity.EditorPropertiesComponent::is_selected)
	else
		print("No instance component")
	end
else
	print("No hit")
end