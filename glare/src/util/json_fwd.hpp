#pragma once

#include <nlohmann/json_fwd.hpp>

namespace util
{
	//using json = nlohmann::json;
	//using json = nlohmann::basic_json<nlohmann::fifo_map>;
	using json = nlohmann::ordered_json;
}