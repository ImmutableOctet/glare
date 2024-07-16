#pragma once

#include <engine/meta/types.hpp>

#define GLARE_IMPL_DECLARE_REFLECT_ENUM()                                   \
	template <typename EnumType>                                            \
	void reflect_enum(bool values_as_properties=false);                     \
	                                                                        \
	template <typename EnumType, bool generate_optional_reflection=true>    \
	auto reflect_enum(MetaTypeID type_id, bool values_as_properties=false);

namespace engine
{
	GLARE_IMPL_DECLARE_REFLECT_ENUM();
}