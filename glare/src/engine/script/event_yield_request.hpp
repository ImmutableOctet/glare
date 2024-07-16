#pragma once

#include <engine/meta/types.hpp>

namespace engine
{
	struct EventYieldRequest
	{
		MetaTypeID event_type_id = {};

		auto operator<=>(const EventYieldRequest&) const = default;
	};

	using ScriptEventYieldRequest = EventYieldRequest;
}