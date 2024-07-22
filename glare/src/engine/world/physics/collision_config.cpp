#include "collision_config.hpp"

#include <engine/entity_type.hpp>

namespace engine
{
	CollisionGroup CollisionConfig::resolve_collision_group(EntityType type)
	{
		switch (type)
		{
			case EntityType::Geometry: return CollisionGroup::StaticGeometry;
			
			case EntityType::Platform: return CollisionGroup::DynamicGeometry;
			case EntityType::Crusher: return CollisionGroup::DynamicGeometry;
			
			case EntityType::Bone: return CollisionGroup::Bone;

			case EntityType::Object: return CollisionGroup::Object;

			// TODO: Determine which group is most suitable for cameras.
			case EntityType::Camera: return CollisionGroup::Actor; // CollisionGroup::Object;
			
			case EntityType::Player: return CollisionGroup::Actor;
			case EntityType::Enemy: return CollisionGroup::Actor;
			case EntityType::FriendlyActor: return CollisionGroup::Actor;

			case EntityType::Collectable: return CollisionGroup::Object;
			
			case EntityType::Particle: return CollisionGroup::Particle; // CollisionGroup::All;
			
			case EntityType::WaterZone: return CollisionGroup::Zone;
			case EntityType::KillZone: return CollisionGroup::Zone;
			case EntityType::DamageZone: return CollisionGroup::Zone;
			case EntityType::EventTrigger: return CollisionGroup::Zone;

			case EntityType::Projectile: return CollisionGroup::Projectile;

			//case EntityType::Light: return CollisionGroup::StaticGeometry;
			//case EntityType::Pivot: return CollisionGroup::None;
			//case EntityType::Regulator: return CollisionGroup::None;
			//case EntityType::Generator: return CollisionGroup::None;

			//case EntityType::Default: return CollisionGroup::None;
		}

		return CollisionGroup::None;
	}

	CollisionGroup CollisionConfig::resolve_solid_mask(EntityType type)
	{
		switch (type)
		{
			case EntityType::Geometry: return CollisionGroup::GeometrySolids;
			case EntityType::Platform: return CollisionGroup::GeometrySolids;
			case EntityType::Crusher: return CollisionGroup::GeometrySolids; // (CollisionGroup::Actor | CollisionGroup::Projectile);
			//case EntityType::Light: return CollisionGroup::None;
			//case EntityType::Pivot: return CollisionGroup::None;
			case EntityType::Bone: return CollisionGroup::BoneSolids;
			case EntityType::Object: return CollisionGroup::ObjectSolids;
			case EntityType::Player: return CollisionGroup::ActorSolids;
			case EntityType::Camera: return CollisionGroup::ObjectSolids; // CollisionGroup::None;
			case EntityType::Enemy: return CollisionGroup::ActorSolids;
			case EntityType::FriendlyActor: return CollisionGroup::ActorSolids;
			case EntityType::Collectable: return CollisionGroup::AllGeometry;
			case EntityType::Particle: return CollisionGroup::AllGeometry; // CollisionGroup::ObjectSolids;
			case EntityType::Projectile: return CollisionGroup::ProjectileSolids; // CollisionGroup::None
			//case EntityType::EventTrigger: return CollisionGroup::None;

			case EntityType::WaterZone: return CollisionGroup::None;
			case EntityType::KillZone: return CollisionGroup::None;
			case EntityType::DamageZone: return CollisionGroup::None;

			case EntityType::Generator: return CollisionGroup::AllGeometry; // CollisionGroup::None

			//case EntityType::Default: return CollisionGroup::None;
		}

		return CollisionGroup::None;
	}

	CollisionGroup CollisionConfig::resolve_interaction_mask(EntityType type)
	{
		switch (type)
		{
			case EntityType::Geometry: return CollisionGroup::None;
			case EntityType::Platform: return CollisionGroup::ObjectInteractions;
			case EntityType::Crusher: return (CollisionGroup::Actor | CollisionGroup::Object | CollisionGroup::AllGeometry);
			//case EntityType::Light: return CollisionGroup::None;
			//case EntityType::Pivot: return CollisionGroup::All;
			//case EntityType::Bone: return CollisionGroup::None;
			case EntityType::Object: return CollisionGroup::ObjectInteractions;
			case EntityType::Player: return CollisionGroup::PlayerInteractions;
			
			// TODO: Determine which interactions cameras should be involved with.
			case EntityType::Camera: return CollisionGroup::ObjectInteractions;

			case EntityType::Enemy: return CollisionGroup::EnemyInteractions;
			case EntityType::FriendlyActor: return CollisionGroup::ObjectInteractions;
			case EntityType::Collectable: return CollisionGroup::CollectableInteractions;
			case EntityType::Particle: return CollisionGroup::AllGeometry;
			case EntityType::Projectile: return CollisionGroup::HitDetectionInteractions;
			case EntityType::EventTrigger: return CollisionGroup::Actor;

			case EntityType::WaterZone: return CollisionGroup::All;
			case EntityType::KillZone: return CollisionGroup::All;
			case EntityType::DamageZone: return CollisionGroup::All;

			case EntityType::Generator: return CollisionGroup::None;

			//case EntityType::Default: return CollisionGroup::None;
		}

		return CollisionGroup::None;
	}

	CollisionConfig::CollisionConfig
	(
		CollisionGroup group,
		CollisionGroup solid_mask,
		CollisionGroup interaction_mask
	) :
		group(group),
		solid_mask(solid_mask),
		interaction_mask(interaction_mask)
	{}

	CollisionConfig::CollisionConfig
	(
		CollisionGroup group,
		CollisionGroup solid_mask,
		CollisionGroup interaction_mask,

		bool enabled
	) : CollisionConfig()
	{
		if (enabled)
		{
			this->group = group;
			this->solid_mask = solid_mask;
			this->interaction_mask = interaction_mask;
		}
	}

	CollisionConfig::CollisionConfig(EntityType type) :
		group(resolve_collision_group(type)),
		solid_mask(resolve_solid_mask(type)),
		interaction_mask(resolve_interaction_mask(type))
	{}

	CollisionConfig::CollisionConfig(EntityType type, bool enabled)
		: CollisionConfig((enabled) ? type : EntityType::Default)
	{}

	bool CollisionConfig::enabled() const
	{
		return
		(group != CollisionGroup::None)
		||
		(solid_mask != CollisionGroup::None)
		||
		(interaction_mask != CollisionGroup::None)
		;
	}
}