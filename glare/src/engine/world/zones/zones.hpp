#pragma once

#include <engine/types.hpp>
#include <engine/multi_types.hpp>

#include <engine/world/world_system.hpp>

#include <math/aabb.hpp>

#include <variant>
#include <vector>

namespace engine
{
	class World;

	using ZoneTarget = std::variant<Entity, Entities, EntityType, EntityTypes>;
	using ZoneCaptures = EntityOrEntities;

	struct ZoneRule
	{
		ZoneTarget target;

		explicit operator bool() const;
	};

	struct ZoneRules
	{
		enum class Mode : std::uint8_t
		{
			Any,
			All
		};

		std::vector<ZoneRule> rules;
		Mode mode;
	};

	using ZoneRuleStorage = std::variant<ZoneRule, ZoneRules>;

	struct ZoneComponent
	{
		bool activated = false;
	};

	class ZoneSystem : public WorldSystem
	{
		public:
			ZoneSystem(World& world);

			inline World& get_world() const { return world; }
		protected:
			void on_subscribe(World& world) override;
			void on_update(World& world, float delta) override;

			World& world;
	};

	Entity create_zone(World& world, const math::AABB& bounds, EntityType type=EntityType::EventTrigger, Entity parent=null, bool update_transform=true);
	Entity create_zone(World& world, const math::Vector extents, EntityType type=EntityType::EventTrigger, Entity parent=null);
}