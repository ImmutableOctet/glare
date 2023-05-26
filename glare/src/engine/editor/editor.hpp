#pragma once

#include <engine/types.hpp>
#include <engine/basic_system.hpp>

namespace engine
{
	class ResourceManager;
	struct Config;

	class Editor : public BasicSystem
	{
		public:
			Editor
			(
				Service& service,
				const Config& config,
				ResourceManager& resource_manager,
				bool subscribe_immediately=false
			);

			virtual ~Editor();

			bool on_subscribe(Service& service) override;

			Entity get_root() const;

		protected:
			ResourceManager& resource_manager;

			Entity instance = null;
	};
}