#pragma once

namespace engine
{
	struct MetaTypeReflectionConfig
    {
        bool capture_standard_data_members         : 1 = true;
        bool generate_optional_reflection          : 1 = true;
        bool generate_operator_wrappers            : 1 = true;
        bool generate_indirect_getters             : 1 = true;
        bool generate_indirect_setters             : 1 = true;
        bool generate_json_bindings                : 1 = true;
        bool generate_binary_bindings              : 1 = true;
        bool generate_history_component_reflection : 1 = true;
    };
}