#pragma once

#include <engine/meta/types.hpp>

#include <util/small_vector.hpp>
//#include <vector>

namespace engine
{
	struct LifetimeDeltaComponent
	{
		using ContainerType = util::small_vector<MetaTypeID, 4>; // std::vector<MetaTypeID>

		bool add_created(MetaTypeID component_type_id);
		bool remove_created(MetaTypeID component_type_id);

		bool add_destroyed(MetaTypeID component_type_id);
		bool remove_destroyed(MetaTypeID component_type_id);

		void clear_created();
		void clear_destroyed();

		void clear();

		bool created(MetaTypeID component_type_id) const;
		bool destroyed(MetaTypeID component_type_id) const;

		const ContainerType& get_created_components() const
		{
			return created_components;
		}
		
		const ContainerType& get_destroyed_components() const
		{
			return destroyed_components;
		}

		ContainerType created_components;
		ContainerType destroyed_components;
	};
}