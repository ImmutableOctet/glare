#include "player_system.hpp"

#include <engine/service.hpp>

namespace engine
{
	PlayerSystem::PlayerSystem(Service& service, bool subscribe_immediately) :
		BasicSystem(service, subscribe_immediately)
	{}

	bool PlayerSystem::on_subscribe(Service& service)
	{
		auto& registry = service.get_registry();

		// Registry events:
		//registry.on_update<TransformComponent>().connect<&PlayerSystem::on_transform_changed>(*this);

		// Standard events:
		service.register_event<OnServiceUpdate, &PlayerSystem::on_update>(*this);

		return true;
	}

	bool PlayerSystem::on_unsubscribe(Service& service)
	{
		service.unregister(*this);

		return true;
	}

	void PlayerSystem::on_update(const OnServiceUpdate& data)
	{
		
	}
}