#pragma once

// TODO: Look into reworking/replacing this header with something more cohesive.

//#include "types.hpp"

#include "entity.hpp"

//#include "physics/collision_group.hpp"
#include "physics/collision_config.hpp"

#include <graphics/types.hpp>
#include <util/enum_operators.hpp>

#include <cstdint>
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

	Entity create_model(World& world, const std::shared_ptr<graphics::Model>& model, Entity parent=null, EntityType type=EntityType::Geometry);
	Entity attach_model(World& world, Entity entity, const std::shared_ptr<graphics::Model>& model, graphics::ColorRGBA color={1.0f, 1.0f, 1.0f, 1.0f}, std::optional<bool> update_name=std::nullopt);

	Entity load_model_attachment
	(
		World& world, Entity entity,
		const std::string& path, // std::string_view path,

		bool allow_multiple=true,

		std::optional<CollisionConfig> collision_cfg=std::nullopt,
		float mass=0.0f,

		const std::shared_ptr<graphics::Shader>& shader={}
	);

	Entity load_model
	(
		World& world,
		const std::string& path, // std::string_view path,

		Entity parent=null,
		EntityType type=EntityType::Geometry,
		
		bool allow_multiple=true,

		std::optional<CollisionConfig> collision_cfg=std::nullopt,
		float mass=0.0f,

		const std::shared_ptr<graphics::Shader>& shader={}
	);

	Entity create_cube(World& world, Entity parent=null, EntityType type=EntityType::Default);
}