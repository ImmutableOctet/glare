#pragma once

#include <engine/types.hpp>
#include <engine/basic_system.hpp>

namespace engine
{
	struct OnServiceUpdate;

	class PlayerSystem : public BasicSystem
	{
		public:
			PlayerSystem(Service& service, bool subscribe_immediately=false);

		protected:
			bool on_subscribe(Service& service) override;
			bool on_unsubscribe(Service& service) override;

			void on_update(const OnServiceUpdate& data);
	};
}