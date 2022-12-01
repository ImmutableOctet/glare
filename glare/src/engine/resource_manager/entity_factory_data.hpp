#pragma once

#include <engine/types.hpp>
#include <engine/entity_factory.hpp>

#include <util/small_vector.hpp>
//#include <string>

namespace engine
{
	using EntityFactoryKey = EntityFactory::FactoryKey; // std::string; // std::filsystem::path;
	using EntityFactoryChildren = util::small_vector<EntityFactoryKey, 2>; // 4 // 8

	struct EntityFactoryData
	{
		using FactoryKey = EntityFactoryKey;

		EntityFactory factory;
		EntityFactoryChildren children;

		Entity create(const EntityConstructionContext& context) const;
	};
}