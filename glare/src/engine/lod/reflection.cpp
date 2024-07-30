#include "reflection.hpp"

#include "entity_batch_system.hpp"
#include "update_level.hpp"

#include "components/level_of_detail_component.hpp"

namespace engine
{
	template <>
	void reflect<LevelOfDetailComponent>()
	{
		engine_meta_type<LevelOfDetailComponent>()
			.data<&LevelOfDetailComponent::update_level>("update_level"_hs)
		;
	}

	template <>
	void reflect<EntityBatchSystem>()
	{
		reflect<UpdateLevel>();

		auto batch_system = engine_system_type<EntityBatchSystem>()
			// ...
		;

		reflect<LevelOfDetailComponent>();
	}
}