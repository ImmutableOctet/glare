#pragma once

#include "types.hpp"
//#include "world/world.hpp"
#include "world/world_system.hpp"

#include <string>
#include <string_view>
#include <optional>
#include <functional>

namespace app
{
	class Window;
}

namespace game
{
	class Game;
}

namespace engine
{
	struct OnStageLoaded;
	struct OnEntityCreated;
	struct OnParentChanged;
	struct OnGravityChanged;
	struct OnTransformChanged;
	struct OnAABBOverlap;
	struct OnCollision;
	struct OnKinematicInfluence;
	struct OnKinematicAdjustment;

	void print_children(World& registry, Entity entity, bool recursive=true, bool summary_info=true, bool recursive_labels=false, const std::string& prefix="->");
	void position_in_titlebar(game::Game& game, Entity entity, std::optional<std::string_view> prefix=std::nullopt);
	
	class DebugListener : public WorldSystem
	{
		public:
			DebugListener(World& world);

			void on_subscribe(World& world) override;

			void operator()(const OnStageLoaded& data);
			void operator()(const OnEntityCreated& data);
			void operator()(const OnParentChanged& data);
			void operator()(const OnGravityChanged& data);
			void operator()(const OnTransformChanged& data);
			void operator()(const OnAABBOverlap& data);
			void operator()(const OnCollision& data);
			void operator()(const OnKinematicInfluence& data);
			void operator()(const OnKinematicAdjustment& data);

			void on_skeleton(Registry& registry, Entity entity);

			std::string label(Entity entity);

			Registry& get_registry() const;
		private:
			template <typename EventType>
			void enable();
	};
}