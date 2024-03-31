#pragma once

#include <engine/script/script_fiber.hpp>
#include <engine/script/script_handle.hpp>

#include <utility>

namespace engine
{
	struct EntityThreadFiber
	{
		ScriptFiber fiber   = {};
		ScriptHandle script = {};

		bool exists() const
		{
			return fiber.exists();
		}

		bool has_script_handle() const
		{
			return static_cast<bool>(script);
		}

		bool done() const
		{
			return fiber.done();
		}

		decltype(auto) operator()(auto&&... args)
		{
			return fiber(std::forward<decltype(args)>(args)...);
		}

		explicit operator bool() const
		{
			return static_cast<bool>(fiber);
		}
	};
}