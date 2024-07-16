#pragma once

#include "script_fiber.hpp"

#include "script_state_interface.hpp"

// Retains the `parameter` specified.
#define GLARE_SCRIPT_ENTRYPOINT_NAMED_PARAMETER(parameter) parameter

// Removes the `parameter` specified.
#define GLARE_SCRIPT_ENTRYPOINT_UNNAMED_PARAMETER(parameter)

// Retains the `type` specified.
#define GLARE_SCRIPT_ENTRYPOINT_NAMED_TYPE(type) type

// Removes the `type` specified.
#define GLARE_SCRIPT_ENTRYPOINT_UNNAMED_TYPE(type)

// Retains the `type` specified, wrapped in `std::declval`.
#define GLARE_SCRIPT_ENTRYPOINT_DECLVAL(type) \
	std::declval<type>()

#define GLARE_SCRIPT_ENTRYPOINT_PARAMETER_LIST_EX(type_name, param_name)            \
        type_name(engine::Script&)                      param_name(self),           \
                                                                                    \
        type_name(const engine::Script&)                param_name(const_self),     \
                                                                                    \
        type_name(engine::Registry&)                    param_name(registry),       \
        type_name(const engine::Entity)                 param_name(entity),         \
        type_name(const engine::MetaEvaluationContext)  param_name(context),        \
                                                                                    \
        type_name(engine::Service&)                     param_name(service),        \
        type_name(engine::SystemManagerInterface&)      param_name(system_manager), \
                                                                                    \
        type_name(engine::World&)                       param_name(world),          \
        type_name(engine::DeltaSystem&)                 param_name(delta_system),   \
                                                                                    \
		type_name(engine::ScriptCurrentStateInterface)  param_name(state),          \
		type_name(engine::ScriptPreviousStateInterface) param_name(prev_state)

#define GLARE_SCRIPT_ENTRYPOINT_PARAMETER_LIST \
	GLARE_SCRIPT_ENTRYPOINT_PARAMETER_LIST_EX(GLARE_SCRIPT_ENTRYPOINT_NAMED_TYPE, GLARE_SCRIPT_ENTRYPOINT_NAMED_PARAMETER)

#define GLARE_SCRIPT_ENTRYPOINT_PARAMETER_LIST_TYPES \
	GLARE_SCRIPT_ENTRYPOINT_PARAMETER_LIST_EX(GLARE_SCRIPT_ENTRYPOINT_NAMED_TYPE, GLARE_SCRIPT_ENTRYPOINT_UNNAMED_PARAMETER)

#define GLARE_SCRIPT_ENTRYPOINT_PARAMETER_LIST_NAMES \
	GLARE_SCRIPT_ENTRYPOINT_PARAMETER_LIST_EX(GLARE_SCRIPT_ENTRYPOINT_UNNAMED_TYPE, GLARE_SCRIPT_ENTRYPOINT_NAMED_PARAMETER)

#define GLARE_SCRIPT_ENTRYPOINT_PARAMETER_LIST_DECLVALS \
	GLARE_SCRIPT_ENTRYPOINT_PARAMETER_LIST_EX(GLARE_SCRIPT_ENTRYPOINT_DECLVAL, GLARE_SCRIPT_ENTRYPOINT_UNNAMED_PARAMETER)

#define GLARE_SCRIPT_ENTRYPOINT_PARAMETERS              \
	(                                                   \
		GLARE_SCRIPT_ENTRYPOINT_PARAMETER_LIST          \
	)

#define GLARE_SCRIPT_ENTRYPOINT_PARAMETERS_EX(...)      \
	(                                                   \
		GLARE_SCRIPT_ENTRYPOINT_PARAMETER_LIST,         \
		__VA_ARGS__                                     \
	)

namespace engine
{
	class Script;

	class Service;
	class SystemManagerInterface;
	class World;
	class DeltaSystem;

	struct MetaEvaluationContext;

	using GLARE_SCRIPT_ENTRYPOINT_FUNCTION = engine::ScriptFiber(*)(GLARE_SCRIPT_ENTRYPOINT_PARAMETER_LIST_TYPES);
}

namespace glare
{
	using GLARE_SCRIPT_ENTRYPOINT_FUNCTION = engine::GLARE_SCRIPT_ENTRYPOINT_FUNCTION;
}