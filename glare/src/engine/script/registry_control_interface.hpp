#pragma once

#include <engine/types.hpp>

namespace engine
{
	class RegistryControlInterface
	{
		public:
			RegistryControlInterface() = default;

			RegistryControlInterface(Registry& registry) :
				_registry(&registry)
			{}

			RegistryControlInterface(const RegistryControlInterface&) = delete;
			RegistryControlInterface(RegistryControlInterface&&) noexcept = default;

			RegistryControlInterface& operator=(const RegistryControlInterface&) = delete;
			RegistryControlInterface& operator=(RegistryControlInterface&&) noexcept = default;

			Registry& get_registry() const
			{
				assert(_registry);

				return *_registry;
			}

			explicit operator bool() const
			{
				return (static_cast<bool>(_registry));
			}

		protected:
			void set_registry(Registry& registry)
			{
				_registry = &registry;
			}

			// The registry this script will use to interact with entities.
			Registry* _registry = {};
	};
}