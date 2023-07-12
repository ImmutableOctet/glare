#pragma once

#include <engine/meta/types.hpp>

#include <util/small_vector.hpp>

//#include <vector>

namespace engine
{
	// Used to determine which components have changed since the last 'delta snapshot' was taken.
	struct DeltaComponent
	{
		using ContainerType = util::small_vector<MetaTypeID, 6>; // std::vector<MetaTypeID>;

		bool add_modified(MetaTypeID component_type_id);
		bool remove_modified(MetaTypeID component_type_id);

		void clear_modified();

		void clear();

		bool modified(MetaTypeID component_type_id) const;

		inline std::size_t size() const
		{
			return modified_components.size();
		}

		inline const ContainerType& get_modified_components() const
		{
			return modified_components;
		}

		ContainerType modified_components;
	};
}