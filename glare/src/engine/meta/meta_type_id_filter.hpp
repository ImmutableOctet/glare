#pragma once

#include "types.hpp"

#include "meta_id_filter.hpp"

namespace engine
{
	template <std::size_t local_buffer_size=6>
	using MetaTypeIDFilter = MetaIDFilter<MetaTypeID, local_buffer_size>;
}