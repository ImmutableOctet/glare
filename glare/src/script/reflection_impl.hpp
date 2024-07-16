#pragma once

#include "script_static_decl.hpp"

#include <engine/script/script.hpp>
#include <engine/reflection/script.hpp>
#include <engine/meta/hash.hpp>

#include <util/fixed_string.hpp>

#include <entt/meta/meta.hpp>

// Debugging related:
//#include <iostream>

namespace glare
{
    namespace impl
    {
        template
        <
            typename ScriptType,

            util::fixed_string glare_script_name,
            util::fixed_string glare_script_local_file_no_ext,

            util::fixed_string glare_script_file_path,
            util::fixed_string glare_script_file_path_without_extension,

            util::fixed_string glare_script_engine_file_path,
            util::fixed_string glare_script_engine_file_path_without_extension
        >
	    auto glare_script_reflect_impl()
        {
            using namespace engine::literals;

            static constexpr auto script_name_id                           = engine::hash(glare_script_name);
                    
            static constexpr auto script_local_name_id                     = engine::hash(glare_script_local_file_no_ext);

            static constexpr auto script_file_id                           = engine::hash(glare_script_engine_file_path);
            static constexpr auto script_file_no_extension_id              = engine::hash(glare_script_engine_file_path_without_extension);
                    
            static constexpr auto script_file_no_base_path_id              = engine::hash(glare_script_file_path);
            static constexpr auto script_file_no_base_path_no_extension_id = engine::hash(glare_script_file_path_without_extension);

            static constexpr auto script_impl_inplace    = static_cast<glare::ScriptFiber(*)(engine::Script&, engine::Registry&, engine::Entity, const engine::MetaEvaluationContext&)>(&glare::script<script_file_id>);
            static constexpr auto script_impl_standalone = static_cast<glare::ScriptFiber(*)(engine::Registry&, engine::Entity, const engine::MetaEvaluationContext&)>(&glare::script<script_file_id>);
            static constexpr auto script_impl_handle     = static_cast<glare::ScriptFiber(*)(engine::ScriptHandle&, engine::Registry&, engine::Entity, const engine::MetaEvaluationContext&)>(&glare::script<script_file_id>);

            static constexpr auto script_impl_no_base_path_inplace    = static_cast<glare::ScriptFiber(*)(engine::Script&, engine::Registry&, engine::Entity, const engine::MetaEvaluationContext&)>(&glare::script<script_file_no_base_path_id>);
            static constexpr auto script_impl_no_base_path_standalone = static_cast<glare::ScriptFiber(*)(engine::Registry&, engine::Entity, const engine::MetaEvaluationContext&)>(&glare::script<script_file_no_base_path_id>);
            static constexpr auto script_impl_no_base_path_handle     = static_cast<glare::ScriptFiber(*)(engine::ScriptHandle&, engine::Registry&, engine::Entity, const engine::MetaEvaluationContext&)>(&glare::script<script_file_no_base_path_id>);

            auto reflect_type = [&](auto& type)
            {
                type = type
                    //.func<script_impl_inplace>(script_file_id)
                    //.func<script_impl_inplace>(script_file_no_extension_id)
                    //.func<script_impl_inplace>(script_name_id)
                    //
                    //.func<script_impl_standalone>(script_file_id)
                    //.func<script_impl_standalone>(script_file_no_extension_id)
                    //.func<script_impl_standalone>(script_name_id)
                    //
                    .func<script_impl_handle>(script_file_id)
                    .func<script_impl_handle>(script_file_no_extension_id)
                    .func<script_impl_handle>(script_name_id)
                    //
                    //.func<script_impl_no_base_path_inplace>(script_file_no_base_path_id)
                    //.func<script_impl_no_base_path_inplace>(script_file_no_base_path_no_extension_id)
                    //
                    //.func<script_impl_no_base_path_standalone>(script_file_no_base_path_id)
                    //.func<script_impl_no_base_path_standalone>(script_file_no_base_path_no_extension_id)
                    //
                    .func<script_impl_no_base_path_handle>(script_file_no_base_path_id)
                    .func<script_impl_no_base_path_handle>(script_file_no_base_path_no_extension_id)
                    //
                    // NOTE: `script_local_name_id` may be ambiguous due to name conflicts.
                    //.func<script_impl_standalone>(script_local_name_id)
                    .func<script_impl_handle>(script_local_name_id)
                ;

                return type;
            };

		    auto shared_type = entt::meta<engine::ScriptNamespace>();

            reflect_type(shared_type);

            auto script_type = entt::meta<ScriptType>()
                .base<engine::Script>()
                .type(script_file_no_extension_id) // script_name_id

                .func<script_impl_inplace>("main"_hs)
                .func<script_impl_standalone>("main"_hs)
                .func<script_impl_handle>("main"_hs)

                .prop("script_cpp"_hs)
                .prop("script"_hs)
            ;

            reflect_type(script_type);

#if GLARE_SCRIPT_IS_COROUTINE
            script_type = script_type.prop("coroutine"_hs);
#endif // GLARE_SCRIPT_IS_COROUTINE

            // Debugging related:
		    //std::cout << "Reflecting script file: '${glare_script_file_path}'" << '\n';
        }
    }
}