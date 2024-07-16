#pragma once

#include <engine/types.hpp>
#include <engine/entity/entity_target.hpp>

#include <utility>

namespace engine
{
	struct EntityTargetComponent
	{
		using Target = EntityTarget;
		using TargetType = EntityTarget::TargetType;

		Target target;

		decltype(auto) get(auto&&... args) const
		{
			return target.get(std::forward<decltype(args)>(args)...);
		}

		decltype(auto) entity(auto&&... args) const
		{
			return get(std::forward<decltype(args)>(args)...);
		}

		bool exists(auto&&... args) const
		{
			return (get(std::forward<decltype(args)>(args)...) != null);
		}

		decltype(auto) operator()(auto&&... args) const
		{
			return get(std::forward<decltype(args)>(args)...);
		}

		inline operator Target() const
		{
			return target;
		}

		inline operator TargetType() const
		{
			return static_cast<TargetType>(target);
		}
	};
	
	using TargetComponent = EntityTargetComponent;
}