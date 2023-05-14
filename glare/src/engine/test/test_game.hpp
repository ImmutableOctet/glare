#pragma once

#include <engine/types.hpp>
#include <engine/registry.hpp>
#include <engine/config.hpp>
#include <engine/delta_time.hpp>

#include <engine/system_manager.hpp>
#include <engine/world/world.hpp>
#include <engine/resource_manager/resource_manager.hpp>

#include <app/types.hpp>

namespace engine
{
	// Basic game-like environment for unit-testing.
	class TestGame
	{
		public:
			using WorldSystemManager = SystemManager<World>;

			inline static constexpr DeltaTime::Rate EXPECTED_UPDATE_RATE = 60;
			inline static constexpr app::Milliseconds EXPECTED_TIME_STEP = static_cast<app::Milliseconds>(1000.0f / static_cast<float>(EXPECTED_UPDATE_RATE));

			TestGame();

			// TODO: Replace with something better.
			template <typename SystemType, typename...Args>
			inline SystemType& system(Args&&... args) { return systems.emplace_system<SystemType>(std::forward<Args>(args)...); }

			// TODO: Replace with something better.
			template <typename SystemType>
			inline SystemType& system() { return systems.emplace_system<SystemType>(); }

			// TODO: Replace with something better.
			template <typename WorldSystemType, typename...Args>
			inline WorldSystemType& world_system(Args&&... args) { return systems.emplace_system<WorldSystemType>(world, std::forward<Args>(args)...); }

			// TODO: Replace with something better.
			template <typename WorldSystemType>
			inline WorldSystemType& world_system() { return systems.emplace_system<WorldSystemType>(world); }

			// TODO: Replace with something better.
			template <typename BehaviorType>
			inline void behavior() { systems.register_behavior<BehaviorType>(); }

			inline Registry& get_registry()
			{
				return registry;
			}

			inline WorldSystemManager& get_systems()
			{
				return systems;
			}

			inline ResourceManager& get_resource_manager()
			{
				return resource_manager;
			}

			inline World& get_world()
			{
				return world;
			}

			inline const DeltaTime& get_delta_time() const
			{
				return world.get_delta_time();
			}

			void update(bool step_forward=true);
			void fixed_update(bool step_forward=false); // true

		protected:
			Registry registry;
			Config cfg;
			ResourceManager resource_manager;
			WorldSystemManager systems;
			World world;

		private:
			app::Milliseconds progress_time();

			app::Milliseconds time = 0;
	};
}