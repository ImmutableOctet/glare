#pragma once

#include "types.hpp"

#include <util/small_vector.hpp>

namespace engine
{
	using MetaIDStorage = util::small_vector<MetaTypeID, 4>;

	using MetaRemovalDescription = MetaIDStorage; // <MetaType>
	using MetaStorageDescription = MetaIDStorage; // <MetaType>
}