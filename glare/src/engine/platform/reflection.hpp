#pragma once

#include <string_view>

namespace engine
{
	bool reflect_exported_functions(std::string_view module_name={});
}