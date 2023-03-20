#pragma once

#include "types.hpp"

#include <optional>

namespace engine
{
	struct MetaVariable;

	class MetaVariableStorageInterface
	{
		public:
			virtual std::optional<std::size_t> get_index(MetaSymbolID name) const = 0;
			
			virtual bool contains(MetaSymbolID name) const = 0;

			virtual const MetaAny* get(MetaSymbolID name) const = 0;
			virtual MetaAny* get(MetaSymbolID name) = 0;

			virtual MetaAny* set(MetaVariable&& variable) = 0;
			virtual MetaAny* set(MetaSymbolID name, MetaAny&& value) = 0;

			virtual bool set_existing(MetaSymbolID name, MetaAny&& value) = 0;
			virtual bool set_missing(MetaSymbolID name, MetaAny&& value) = 0;

			//virtual const MetaAny* data() const = 0;
			virtual std::size_t size() const = 0;
	};
}