#pragma once

//#include "types.hpp"
#include <engine/events.hpp>

//#include <string>
#include <optional>
#include <filesystem>

namespace engine
{
	struct TransformComponent;

	// Indicates that a stage/map has been loaded into the `World`, and that its parent is `stage`.
	struct OnStageLoaded
	{
		// The parent/pivot entity representing the loaded stage.
		Entity stage;

		// An optional file-path indicating the source of the stage in question.
		std::optional<std::filesystem::path> path = std::nullopt;
	};

	// Indicates that a player object has finished loading.
	struct OnPlayerLoaded
	{
		Entity player;

		// An optional file-path indicating the definition for the player's character.
		std::optional<std::filesystem::path> path = std::nullopt;
	};

	/*
		Indicates that the transformation of an entity has changed in some way.
		This can occur due to parent entities changing their transformations as well.
		
		Likewise, small modifications to the entity's transform will also trigger this event.
		e.g. minor differences in rotation caused by billboard behaviors, mouse input, etc.
	*/
	struct OnTransformChanged
	{
		Entity entity;
	};

	// This event is triggered any time a `World` object's `set_gravity` command is called.
	struct OnGravityChanged
	{
		math::Vector old_gravity;
		math::Vector new_gravity;
	};
}