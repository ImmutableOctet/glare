#pragma once

#include <engine/time_decl.hpp>
#include <engine/lod/update_level.hpp>

namespace engine
{
	struct OnBatchUpdate
	{
		time::Duration time_elapsed = {};
		
		float delta = 1.0f;

		UpdateLevel batch_update_level = UpdateLevel::Default;
	};
}