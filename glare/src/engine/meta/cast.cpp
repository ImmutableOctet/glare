#include "cast.hpp"

#include "indirection.hpp"

//#include <entt/meta/meta.hpp>

namespace engine
{
	bool try_direct_cast(MetaAny& current_value, const MetaType& cast_type, bool ignore_same_type)
	{
		auto current_type = current_value.type();

		if (ignore_same_type && (current_type == cast_type))
		{
			// No need to cast, the specified cast-type is the same.
			return true;
		}
		else
		{
			if (!type_has_indirection(current_type))
			{
				if (current_value.allow_cast(cast_type))
				{
					return true;
				}
				else if (auto construct_from = cast_type.construct(current_value))
				{
					current_value = construct_from;

					return true;
				}
			}
		}

		return false;
	}
}