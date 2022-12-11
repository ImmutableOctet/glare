#pragma once

#include <engine/types.hpp>
//#include <engine/world/world.hpp>
#include <engine/world/world_system.hpp>

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

namespace app::input
{
	struct OnGamepadConnected;
	struct OnGamepadDisconnected;
	struct OnGamepadButtonDown;
	struct OnGamepadButtonUp;
	struct OnGamepadAnalogInput;
}

namespace engine
{
	// Events:
	struct OnStageLoaded;
	struct OnEntityCreated;
	struct OnParentChanged;
	struct OnGravityChanged;
	struct OnTransformChanged;
	
	struct OnAABBOverlap;
	struct OnCollision;
	struct OnKinematicInfluence;
	struct OnKinematicAdjustment;

	struct OnButtonDown;
	struct OnButtonReleased;
	struct OnButtonPressed;
	struct OnAnalogInput;

	// Commands:
	struct PrintCommand;

	void print_children(World& registry, Entity entity, bool recursive=true, bool summary_info=true, bool recursive_labels=false, const std::string& prefix="->");
	void position_in_titlebar(game::Game& game, Entity entity, std::optional<std::string_view> prefix=std::nullopt);
	
	class DebugListener : public WorldSystem
	{
		public:
			DebugListener(World& world);

			void on_subscribe(World& world) override;
			void on_skeleton(Registry& registry, Entity entity);

			Registry& get_registry() const;

			std::string label(Entity entity);

			void operator()(const OnStageLoaded& data);
			void operator()(const OnEntityCreated& data);
			void operator()(const OnParentChanged& data);
			void operator()(const OnGravityChanged& data);
			void operator()(const OnTransformChanged& data);

			// Collision/physics debugging:
			void operator()(const OnAABBOverlap& data);
			void operator()(const OnCollision& data);
			void operator()(const OnKinematicInfluence& data);
			void operator()(const OnKinematicAdjustment& data);

			// Gamepad debugging:
			void operator()(const app::input::OnGamepadConnected& data);
			void operator()(const app::input::OnGamepadDisconnected& data);
			void operator()(const app::input::OnGamepadButtonDown& data);
			void operator()(const app::input::OnGamepadButtonUp& data);
			void operator()(const app::input::OnGamepadAnalogInput& data);

			// High-level input system:
			void operator()(const OnButtonDown& data);
			void operator()(const OnButtonReleased& data);
			void operator()(const OnButtonPressed& data);
			void operator()(const OnAnalogInput& data);

			void operator()(const PrintCommand& data);
		private:
			template <typename EventType>
			void enable();
	};
}