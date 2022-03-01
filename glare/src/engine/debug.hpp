#pragma once

#include "types.hpp"
//#include "world/world.hpp"

#include <string>
#include <functional>

namespace engine
{
	struct OnStageLoaded;
	struct OnEntityCreated;
	struct OnParentChanged;

	void print_children(World& registry, Entity entity, bool recursive=true, bool summary_info=true, bool recursive_labels=false, const std::string& prefix="->");
	
	class DebugListener
	{
		protected:
			World& world;
		public:
			DebugListener(World& world);
			~DebugListener();

			template <typename EventType>
			void enable();

			void operator()(const OnStageLoaded& data);
			void operator()(const OnEntityCreated& data);
			void operator()(const OnParentChanged& data);

			std::string label(Entity entity);

			inline World& get_world() const
			{
				return world;
			}

			Registry& get_registry() const;
	};
}