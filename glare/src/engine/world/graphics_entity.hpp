#pragma once

//#include "types.hpp"
#include <types.hpp>
#include <graphics/types.hpp>

#include "entity.hpp"
#include "physics/collision_group.hpp"

#include <optional>
#include <string>

namespace graphics
{
	class Model;
	class Shader;
	//class Context;
}

namespace engine
{
	class World;
	class ResourceManager;
	struct AnimationData;

	/*
	enum class RenderFlags : std::uint32_t
	{
		None = 0,

		IsHidden   = (1 << 0),
		CastsShadow = (1 << 1),
	};

	FLAG_ENUM(std::uint32_t, RenderFlags);

	struct RenderFlagsComponent
	{
		using Flags = RenderFlags;

		RenderFlags flags = Flags::None;
		//unsigned short depth_sort = 0;

		inline bool is_visible() const { return !(flags & Flags::IsHidden); }
		inline bool casts_shadow() const { return !(flags & Flags::CastsShadow); }
	};
	*/

	// Entity with 3D Model component.
	Entity create_model(World& world, pass_ref<graphics::Model> model, Entity parent=null, EntityType type=EntityType::Geometry);
	Entity attach_model(World& world, Entity entity, pass_ref<graphics::Model> model, graphics::ColorRGBA color={1.0f, 1.0f, 1.0f, 1.0f});

	Entity load_model
	(
		World& world, const std::string& path, Entity parent=null, EntityType type=EntityType::Geometry, // std::string_view path,
		
		bool allow_multiple=true,
		bool collision_enabled=false, float mass = 0.0f,

		std::optional<CollisionGroup> collision_group=std::nullopt,
		std::optional<CollisionGroup> collision_solid_mask=std::nullopt,
		std::optional<CollisionGroup> collision_interaction_mask=std::nullopt,

		pass_ref<graphics::Shader> shader={}
	);

	Entity create_cube(World& world, Entity parent=null, EntityType type=EntityType::Default);

	Entity attach_animator(World& world, Entity entity, const pass_ref<AnimationData> animations, float rate=1.0f);
}