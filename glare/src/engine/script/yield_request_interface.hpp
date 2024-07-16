#pragma once

#include <engine/meta/types.hpp>

#include <utility>

namespace engine
{
	class Script;

	class YieldRequestInterface
	{
		public:
			virtual ~YieldRequestInterface() = default;

			virtual bool get_boolean_result() const = 0;

			virtual bool operator()() const
			{
				return get_boolean_result();
			}

			virtual bool operator()()
			{
				return (std::as_const(*this))();
			}

			virtual bool operator()(const MetaAny& opaque_value)
			{
				return operator()();
			}

			virtual bool operator()(const Script& self, const MetaAny& opaque_value)
			{
				return operator()(opaque_value);
			}

			explicit operator bool() const
			{
				return get_boolean_result();
			}
	};
}