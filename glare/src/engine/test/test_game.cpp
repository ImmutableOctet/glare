#include "test_game.hpp"

namespace engine
{
	TestGame::TestGame() :
		cfg(),
		resource_manager(nullptr),
		world(registry, systems, cfg, resource_manager, EXPECTED_UPDATE_RATE),
		systems(world)
	{}

	void TestGame::update()
	{
		world.update(SystemClock::now());
	}

	void TestGame::fixed_update()
	{
		world.fixed_update(SystemClock::now());
	}
}