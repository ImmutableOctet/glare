#pragma once

#include "types.hpp"
#include "meta_event_listener_flags.hpp"

#include <optional>

namespace engine
{
	class Service;
	class SystemManagerInterface;
	class MetaEventListener;

	bool connect_listener
	(
		MetaEventListener& listener,
		const MetaType& event_type,
		
		Service& service,
		
		SystemManagerInterface* opt_system_manager=nullptr,
		std::optional<MetaEventListenerFlags> opt_flags=std::nullopt
	);

	bool connect_listener
	(
		MetaEventListener& listener,
		MetaTypeID event_type_id,

		Service& service,

		SystemManagerInterface* opt_system_manager=nullptr,
		std::optional<MetaEventListenerFlags> opt_flags=std::nullopt
	);
}