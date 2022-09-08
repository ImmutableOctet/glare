#pragma once

#include "types.hpp"
//#include "world/world.hpp"

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

	void print_children(World& registry, Entity entity, bool recursive=true, bool summary_info=true, bool recursive_labels=false, const std::string& prefix="->");
	void position_in_titlebar(game::Game& game, Entity entity, std::optional<std::string_view> prefix=std::nullopt);
	
	class DebugListener
	{
		protected:
			World& world;

		private:
			template <typename EventType>
			void enable();

		public:
			DebugListener(World& world);
			~DebugListener();

			void operator()(const OnStageLoaded& data);
			void operator()(const OnEntityCreated& data);
			void operator()(const OnParentChanged& data);

			void on_skeleton(Registry& registry, Entity entity);

			std::string label(Entity entity);

			inline World& get_world() const
			{
				return world;
			}

			Registry& get_registry() const;
	};
}