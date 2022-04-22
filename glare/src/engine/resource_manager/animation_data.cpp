#include "animation_data.hpp"

namespace engine
{
	float AnimationData::get_transition(AnimationID src, AnimationID dest) const
	{
		auto it = transitions.find({ src, dest });

		if (it != transitions.end())
		{
			return it->second;
		}

		return 0.0f;
	}
}