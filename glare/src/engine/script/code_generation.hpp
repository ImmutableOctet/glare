#pragma once

#include "script.hpp"
#include "script_fiber.hpp"

#include <engine/types.hpp>
#include <engine/meta/types.hpp>
#include <engine/meta/hash.hpp>

#include <util/type_traits.hpp>

namespace engine
{
    struct MetaEvaluationContext;

    class Script;
}

namespace game
{
    namespace impl
    {
        class ScriptImpl;
    }
}

namespace glare
{
    namespace impl
    {
        class ScriptImpl;
    }
}

#define GLARE_GENERATE_SCRIPT_TYPE_EX(script_type_name, script_extension_type_name, script_file_path, script_parameter_list) \
    class script_type_name :                                                                                                 \
        public engine::Script,                                                                                               \
                                                                                                                             \
        public util::defined_or_empty_t<glare::impl::ScriptImpl>,                                                            \
        public util::defined_or_empty_t<game::impl::ScriptImpl>,                                                             \
        public util::defined_or_empty_t<script_extension_type_name>                                                          \
    {                                                                                                                        \
        protected:                                                                                                           \
            script_type_name() = default;                                                                                    \
	                                                                                                                         \
        public:                                                                                                              \
            using Base = engine::Script;                                                                                     \
                                                                                                                             \
            using CoreScriptType = Base;                                                                                     \
                                                                                                                             \
            inline static constexpr engine::StringHash script_id              = engine::hash(script_file_path);              \
            inline static constexpr engine::StringHash script_no_base_path_id = engine::hash("engine/" script_file_path);    \
			                                                                                                                 \
            script_type_name                                                                                                 \
			(                                                                                                                \
				engine::Registry&                    registry,                                                               \
				engine::Entity                       entity,                                                                 \
				const engine::MetaEvaluationContext& context                                                                 \
			) :                                                                                                              \
                engine::Script(context, registry, script_id, entity)                                                         \
            {}                                                                                                               \
			                                                                                                                 \
            script_type_name(script_type_name&&) noexcept = default;                                                         \
			                                                                                                                 \
            script_type_name(engine::Script&& storage) noexcept                                                              \
                : script_type_name()                                                                                         \
            {                                                                                                                \
                *this = std::move(storage);                                                                                  \
            }                                                                                                                \
			                                                                                                                 \
            script_type_name& operator=(const script_type_name&) = delete;                                                   \
            script_type_name& operator=(script_type_name&&) noexcept = default;                                              \
			                                                                                                                 \
            script_type_name& operator=(engine::Script&& storage) noexcept                                                   \
            {                                                                                                                \
                engine::Script::operator=(std::move(storage));                                                               \
				                                                                                                             \
                return *this;                                                                                                \
            }                                                                                                                \
			                                                                                                                 \
            using engine::Script::operator();                                                                                \
			                                                                                                                 \
            engine::ScriptFiber operator()(script_parameter_list) override;                                                  \
    };

#define GLARE_GENERATE_SCRIPT_TYPE(script_file_path) \
    GLARE_GENERATE_SCRIPT_TYPE_EX(script_t, script_ex_t, script_file_path, GLARE_SCRIPT_ENTRYPOINT_PARAMETER_LIST)