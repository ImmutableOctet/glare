#include <engine/types.hpp>

#include <variant>
#include <string>
#include <string_view>
#include <unordered_map>

#define GLARE_ENGINE_REFLECT_ALL_AUTOMATICALLY

#ifdef GLARE_ENGINE_REFLECT_ALL_AUTOMATICALLY
	#include "reflect_all.hpp"
#endif

namespace engine
{
	namespace impl
	{
		std::unordered_map<StringHash, std::variant<std::string, std::string_view>> known_string_hashes;

		// NOTE: This is defined in the same file as `known_string_hashes` to avoid out-of-order initialization issues.
#ifdef GLARE_ENGINE_REFLECT_ALL_AUTOMATICALLY
        struct reflect_all_t
        {
            reflect_all_t() { engine::reflect_all(); }
        };

		static reflect_all_t execute_reflect_all {};
#endif
	}
}