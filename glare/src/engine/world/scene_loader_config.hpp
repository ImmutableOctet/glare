#pragma once

namespace engine
{
	// Loader configuration; e.g. should we load objects, players, etc.
	struct SceneLoaderConfig
	{
		bool title                     : 1 = true;
		bool properties                : 1 = true;
		bool geometry                  : 1 = true;
		bool objects                   : 1 = true;
		bool players                   : 1 = true;
		bool apply_transform           : 1 = true;
		bool identify_static_mutations : 1 = false;
	};
}