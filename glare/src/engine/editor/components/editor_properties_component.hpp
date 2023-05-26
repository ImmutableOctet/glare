#pragma once

namespace engine
{
	struct EditorPropertiesComponent
	{
		bool is_selected : 1 = false;

		inline bool get_is_selected() const
		{
			return is_selected;
		}

		inline void set_is_selected(bool value)
		{
			is_selected = value;
		}
	};
}