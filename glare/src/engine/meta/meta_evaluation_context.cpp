#include "meta_evaluation_context.hpp"

#include "hash.hpp"
#include "function.hpp"
#include "runtime_traits.hpp"

#include <engine/service.hpp>
#include <engine/system_manager_interface.hpp>

namespace engine
{
	MetaAny MetaEvaluationContext::resolve_singleton_from_type(const MetaType& type) const
	{
		using namespace engine::literals;

		if (type)
		{
			if (system_manager)
			{
				if (type_is_system(type))
				{
					// NOTE: Const-cast used since requested system could need to be non-const for caller.
					if (auto system_handle = const_cast<SystemManagerInterface*>(system_manager)->get_system_handle(type.id()))
					{
						return *system_handle.ptr;
					}
				}
			}

			if (service)
			{
				if (type_is_service(type))
				{
					if (auto dynamic_cast_fn = type.func("dynamic_cast"_hs))
					{
						// NOTE: Const-cast used since service could need to be non-const for caller.
						if (auto resolved_service_ptr = invoke_any_overload(dynamic_cast_fn, MetaAny {}, const_cast<Service*>(service)))
						{
							return *resolved_service_ptr;
						}
					}
				}
			}
		}

		return {};
	}

	MetaAny MetaEvaluationContext::resolve_singleton_from_type_id(const MetaTypeID type_id) const
	{
		return resolve_singleton_from_type(resolve(type_id));
	}
}