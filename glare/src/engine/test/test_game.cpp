#include "test_game.hpp"

namespace engine
{
	TestGame::TestGame() :
		cfg(),
		resource_manager(nullptr),
		world(registry, systems, cfg, resource_manager, EXPECTED_UPDATE_RATE),
		systems(world)
	{}

	void TestGame::update(bool step_forward)
	{
		world.update(time);

		if (step_forward)
		{
			progress_time();
		}
	}

	void TestGame::fixed_update(bool step_forward)
	{
		world.fixed_update(time);

		if (step_forward)
		{
			progress_time();
		}
	}

	app::Milliseconds TestGame::progress_time()
	{
		time += EXPECTED_TIME_STEP;

		return time;
	}
}