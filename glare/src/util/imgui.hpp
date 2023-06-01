#pragma once

#include <imgui.h>

// ImGui extensions:
namespace ImGui
{
	template <typename ...Args>
	void FormatText(fmt::format_string<Args...> fmt, Args&&... args)
	{
		Text(util::format(fmt, std::forward<Args>(args)...).c_str());
	}
}

namespace util
{
	namespace imgui = ::ImGui;
}