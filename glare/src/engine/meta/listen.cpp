#include "listen.hpp"

#include "meta_event_listener.hpp"

#include "hash.hpp"

// Debugging related:
#include <util/log.hpp>

namespace engine
{
	bool connect_listener
	(
		MetaEventListener& listener,
		const MetaType& event_type,
		
		Service& service,
		
		SystemManagerInterface* opt_system_manager,
		std::optional<MetaEventListenerFlags> opt_flags
	)
	{
		using namespace engine::literals;

		auto connect_fn = event_type.func("connect_meta_event"_hs);

		if (!connect_fn)
		{
			print_error("Unable to resolve connection function for meta event-listener -- type: {}", get_known_string_from_hash(event_type.id()));

			return false;
		}

		auto result = connect_fn.invoke
		(
			{},

			entt::forward_as_meta(listener),
			entt::forward_as_meta(&service),
			entt::forward_as_meta(opt_system_manager),

			entt::forward_as_meta(opt_flags)
		);

		return static_cast<bool>(result);
	}

	bool connect_listener
	(
		MetaEventListener& listener,
		MetaTypeID event_type_id,

		Service& service,

		SystemManagerInterface* opt_system_manager,
		std::optional<MetaEventListenerFlags> opt_flags
	)
	{
		const auto event_type = resolve(event_type_id);

		if (!event_type)
		{
			//print_warn("Unable to resolve meta-type for event: #{}", event_type_id);

			return false;
		}

		return connect_listener
		(
			listener,
			event_type,

			service,
			opt_system_manager,
			opt_flags
		);
	}
}