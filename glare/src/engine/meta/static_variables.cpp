#include <engine/types.hpp>

#include <variant>
#include <string>
#include <string_view>
#include <unordered_map>

#if GLARE_ENGINE_REFLECT_ALL_AUTOMATICALLY
	#include "reflect_all.hpp"
#endif // GLARE_ENGINE_REFLECT_ALL_AUTOMATICALLY

namespace engine
{
	namespace impl
	{
#if GLARE_ENGINE_REFLECT_ALL_AUTOMATICALLY
		reflect_all_t::reflect_all_t() { engine::reflect_all(); }
#endif // GLARE_ENGINE_REFLECT_ALL_AUTOMATICALLY
	}
}

#if GLARE_ENGINE_REFLECT_ALL_AUTOMATICALLY
	const struct GLARE_ENGINE_SYMBOL engine::impl::reflect_all_t glare_impl_engine_execute_reflect_all {};
#endif // GLARE_ENGINE_REFLECT_ALL_AUTOMATICALLY