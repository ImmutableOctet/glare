if (GLARE_SCRIPT_CLING_ENABLED)
    add_library(glare_script OBJECT) # STATIC # SHARED

    #set_target_properties(
    #    glare_script
    #    PROPERTIES
    #    LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}
    #)
else()
    add_library(glare_script OBJECT) # STATIC
endif()

add_library(${PROJECT_NAME}::script ALIAS glare_script)

target_compile_features(glare_script PRIVATE cxx_std_23)

find_package(concurrencpp CONFIG REQUIRED)
target_link_libraries(glare_script PRIVATE concurrencpp::concurrencpp)

#target_compile_definitions(glare_script PUBLIC GLARE_SCRIPT_PRECOMPILED=${GLARE_SCRIPT_PRECOMPILED})
#target_compile_definitions(glare_script PUBLIC GLARE_SCRIPT_CLING_ENABLED=${GLARE_SCRIPT_CLING_ENABLED})

if (GLARE_SCRIPT_CLING_ENABLED)
    target_compile_definitions(glare_script PRIVATE GLARE_SCRIPT_CLING=1)
    #target_compile_definitions(glare_script PUBLIC GLARE_SCRIPT_PRECOMPILED=0)

    #set_target_properties(glare_script PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS 1)

    target_sources(
        glare_script
    
        PRIVATE
    
        "cling.cpp"
    )

    set(GLARE_CLING_USE_THIRDPARTY ON)

    if (GLARE_CLING_USE_THIRDPARTY)
        #set(LLVM_TARGETS_TO_BUILD "host;NVPTX")

        # FetchContent:
        #FetchContent_Declare(
        #  cling
        #  SOURCE_DIR "${CMAKE_SOURCE_DIR}/thirdparty/cling/cling/"
        #  FIND_PACKAGE_ARGS NAMES cling
        #)
        #
        #FetchContent_MakeAvailable(cling)
    
        #find_library(cling
        #    cling clingInterpreter
        #    HINTS "${CMAKE_PREFIX_PATH}/third-party"
        #)

        # Direct usage:
        set(CLING_DIR "${CMAKE_SOURCE_DIR}/third-party/cling")
        set(CLING_PREFIX "${CMAKE_SOURCE_DIR}/third-party/cling/cling")
        set(LLVM_BUILD_BINARY_DIR "${CMAKE_SOURCE_DIR}/third-party/cling/build")
        set(CLING_CLANG_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/third-party/cling/src/clang/include")

        include("${CMAKE_SOURCE_DIR}/third-party/cmake/FindCling.cmake")
        include("${CMAKE_SOURCE_DIR}/third-party/cmake/FindClangForCling.cmake")
    else()
        FetchContent_Declare(
          cling
          GIT_REPOSITORY https://github.com/root-project/cling.git
          GIT_TAG        ab81cdc # v1.0
          FIND_PACKAGE_ARGS NAMES cling
        )

        FetchContent_MakeAvailable(cling)
    endif()
endif()

if(GLARE_SCRIPT_PRECOMPILED)
    #set(GLARE_SCRIPT_PRECOMPILED_GENERATE_CONSUMPTION_FILES ON)
    #set(GLARE_SCRIPT_PRECOMPILED_GENERATE_HEADER_FILES ON)
    ##set(GLARE_SCRIPT_PRECOMPILED_UNITY ON)

    #set_target_properties(glare_script PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS 1)
    set_target_properties(glare_script PROPERTIES ENABLE_EXPORTS 1)

    if (GLARE_SCRIPT_PRECOMPILED_UNITY)
        set_target_properties(
            glare_script
            PROPERTIES
            UNITY_BUILD ON
            UNITY_BUILD_MODE BATCH
        )
    endif()
    
    target_link_libraries(
        glare_script
    
        PRIVATE
   
        glare_core
        glare_engine
    )

    target_compile_definitions(glare_script PRIVATE GLARE_SCRIPT_EXPORT_SYMBOLS=1)

    set(GLARE_RUNTIME_FILES ${CMAKE_SOURCE_DIR}/glare/glare)
    set(GLARE_RUNTIME_ENGINE_FILES ${GLARE_RUNTIME_FILES}/engine)

    #set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR}/glare/glare)
    #set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR}/glare/glare/*.cpp)

    # Enumerate script source files:
    file(
        GLOB_RECURSE GLARE_RUNTIME_SOURCES
        RELATIVE ${GLARE_RUNTIME_ENGINE_FILES}

        CONFIGURE_DEPENDS
        
        ${GLARE_RUNTIME_ENGINE_FILES}/*.cpp
        ${GLARE_RUNTIME_ENGINE_FILES}/*.cxx
        ${GLARE_RUNTIME_ENGINE_FILES}/*.C

        ${GLARE_RUNTIME_ENGINE_FILES}/*.h
        ${GLARE_RUNTIME_ENGINE_FILES}/*.hpp
        ${GLARE_RUNTIME_ENGINE_FILES}/*.hxx
        ${GLARE_RUNTIME_ENGINE_FILES}/*.H
    )

    message(GLARE_RUNTIME_SOURCES="${GLARE_RUNTIME_SOURCES}")

    if (GLARE_SCRIPT_PRECOMPILED_GENERATE_CONSUMPTION_FILES)
        set(glare_consume_all_scripts_hpp_output_file "generated/consume_scripts.hpp")

        set(glare_consume_all_scripts_hpp_content
        "
            #pragma once

        ")
    endif()

    # Currently one file per generated library. (May later group by folder, etc.)
    foreach(glare_script_file_path ${GLARE_RUNTIME_SOURCES})
        if (${glare_script_file_path} MATCHES ".+\.(h|hpp|hxx|H)$")
            message("Skipping code generation for header file: \"${glare_script_file_path}\"")
        else()
            string(REGEX REPLACE "\\.[^.]*$" "" glare_script_file_path_without_extension ${glare_script_file_path})
            string(REGEX REPLACE "[^a-zA-Z\_]+" "_" glare_script_name "${glare_script_file_path_without_extension}")

            #cmake_path(GET ${glare_script_file_path} FILENAME glare_script_local_file)
            get_filename_component(glare_script_local_file ${glare_script_file_path} NAME)

            string(REGEX MATCH "([^\.]+)\..*$" _ ${glare_script_local_file})

            set(glare_script_local_file_no_ext "${CMAKE_MATCH_1}")

            set(glare_script_file_library "glare_script_${glare_script_name}")

            set(glare_script_output_cpp_file "generated/${glare_script_file_library}.cpp")
            set(glare_script_output_hpp_file "generated/${glare_script_file_library}.hpp")
            set(glare_consume_script_output_hpp_file "generated/${glare_script_file_library}_consume.hpp")
            set(glare_consume_script_output_cpp_file "generated/${glare_script_file_library}_consume.cpp")

            message("Generating prebuilt script source file: \"${glare_script_file_path}\" -> \"${glare_script_output_cpp_file}\"")

            set(script_file_content "")
            set(script_file_header_content "")

            if (GLARE_SCRIPT_PRECOMPILED_GENERATE_HEADER_FILES)
                string(PREPEND script_file_header_content "#pragma once\n\n")
            endif()
            
            string(REGEX MATCH "(.+)\.(cpp|cxx|C)$" _ ${glare_script_file_path})
            
            set(glare_script_header_filter_name "${CMAKE_MATCH_1}")

            #message(glare_script_header_filter_name=${glare_script_header_filter_name})

            file(
                GLOB script_header_files

                CONFIGURE_DEPENDS
        
                ${GLARE_RUNTIME_ENGINE_FILES}/${glare_script_header_filter_name}.h
                ${GLARE_RUNTIME_ENGINE_FILES}/${glare_script_header_filter_name}.hpp
                ${GLARE_RUNTIME_ENGINE_FILES}/${glare_script_header_filter_name}.hxx
                ${GLARE_RUNTIME_ENGINE_FILES}/${glare_script_header_filter_name}.H
            )

            #message(script_header_files=${script_header_files})
            
            foreach(script_header ${script_header_files})
                if (GLARE_SCRIPT_EMBED_SOURCE)
                    message("Found header file for script: \"${script_header}\", embedding directly...")

                    file(STRINGS ${script_header} script_file_content_hpp)
                    
                    list(APPEND script_file_header_content "${script_file_content_hpp}" "\n\n")
                else()
                    message("Found header file for script: \"${script_header}\", embedding include...")
                
                    string(APPEND script_file_header_content "#include \"${script_header}\"\n")
                endif()
            endforeach()

            if (GLARE_SCRIPT_EMBED_SOURCE)
                # Read entire file up front:
                file(STRINGS ${GLARE_RUNTIME_ENGINE_FILES}/${glare_script_file_path} script_file_content_cpp)

                string(APPEND script_file_content ${script_file_content_cpp})
            else()
                # Include file instead:
                string(APPEND script_file_content "#include \"${GLARE_RUNTIME_ENGINE_FILES}/${glare_script_file_path}\"\n")
            endif()

            string(COMPARE NOTEQUAL "${script_file_header_content}" "" script_file_header_content_exists)

            if (script_file_header_content_exists)
                string(APPEND script_file_header_content "\n\n")
            endif()

            string(APPEND script_file_header_content
            "
                // Standard includes for `${glare_script_file_library}`:
                #include <script/api.hpp>
                #include <script/script_static_decl.hpp>
                #include <script/script_common.hpp>

                #include <engine/types.hpp>
                #include <engine/script.hpp>

                //#include <engine/meta/hash.hpp>

                #include <utility>
                #include <cassert>

                // Declarations for `${glare_script_file_library}`:
                #define ${glare_script_file_library}_run_decl_params                       \\
                (                                                                          \\
                    glare::impl::${glare_script_file_library}::script_t& self,             \\
                                                                                           \\
                    const glare::impl::${glare_script_file_library}::script_t& const_self, \\
                                                                                           \\
                    engine::Registry& registry,                                            \\
                    const engine::Entity entity,                                           \\
                    const engine::MetaEvaluationContext context,                           \\
                                                                                           \\
                    engine::Service& service,                                              \\
                    engine::SystemManagerInterface& system_manager,                        \\
                                                                                           \\
                    engine::World& world,                                                  \\
                    engine::DeltaSystem& delta_system                                      \\
                )

                namespace glare
                {
                    namespace impl
                    {
                        namespace ${glare_script_file_library}
                        {
                            using namespace glare_script_common;

                            //GLARE_SCRIPT_API
                            class script_t final : public engine::Script
                            {
                                private:
                                    script_t() = default;

                                public:
                                    inline static constexpr NativeScriptID script_id = engine::hash(\"${glare_script_file_path}\");
                                    inline static constexpr NativeScriptID script_no_base_path_id = engine::hash(\"engine/${glare_script_file_path}\");

                                    script_t
			                        (
				                        const MetaEvaluationContext& context,
				                        Registry&                    registry,
				                        ScriptID                     script_id,
				                        Entity                       entity
			                        ) :
                                        engine::Script(context, registry, script_id, entity)
                                    {}

                                    script_t(script_t&&) noexcept = default;
                                    script_t(engine::Script&& storage) noexcept
                                        : script_t()
                                    {
                                        *this = std::move(storage);
                                    }

                                    script_t& operator=(const script_t&) = delete;
                                    script_t& operator=(script_t&&) noexcept = default;
                                
                                    script_t& operator=(engine::Script&& storage) noexcept
                                    {
                                        engine::Script::operator=(std::move(storage));

                                        return *this;
                                    }

                                    glare::ScriptFiber run ${glare_script_file_library}_run_decl_params;

                                    decltype(auto) operator()();
                            };
                        }
                    }
                }
            ")

            set(before_script
            "
                #define _script_no_return_value                              \\
                    glare::impl::${glare_script_file_library}::script_t::run \\
                        ${glare_script_file_library}_run_decl_params
                
                //#define ${glare_script_local_file_no_ext}(...) \\
                //    glare::ScriptFiber _script_no_return_value

                #define ${glare_script_local_file_no_ext}(...) \\
                    _script_no_return_value -> glare::ScriptFiber

                // Too general:
                //#define main(...) \\
                //    glare::ScriptFiber _script_no_return_value

                //#define script(...) \\
                //    glare::ScriptFiber _script_no_return_value

            ")
            #       { Script body }
            set(after_script
            "
                
                //#undef script
                //#undef main
                #undef _script_no_return_value

                #undef ${glare_script_local_file_no_ext}

                namespace glare
                {
                    namespace impl
                    {
                        namespace ${glare_script_file_library}
                        {
                            decltype(auto) script_t::operator()()
                            {
                                return run
                                (
                                    *this,
                                    *this,
                                    get_registry(),
                                    get_entity(),
                                    get_context(),
                                    get_service(),
                                    get_system_manager(),
                                    get_world(),
                                    get_delta_system()
                                );
                            }
                        }
                    }

                    GLARE_SCRIPT_API
                    ScriptFiber ${glare_script_file_library}(engine::Script& self, engine::Registry& registry, engine::Entity entity, const engine::MetaEvaluationContext context)
                    {
                        // Ensure that the derived `script_t` is just for exposing the API, and not adding state.
                        //static_assert(sizeof(engine::Script) == sizeof(impl::${glare_script_file_library}::script_t));

                        if (!self)
                        {
                            // TODO: Look into using an 'any' type here, so we could do something like: self.script = ...;
                            self = impl::${glare_script_file_library}::script_t
                            {
                                context,
                                registry,
                                impl::${glare_script_file_library}::script_t::script_id,
                                entity
                            };

                            assert(self);
                        }

                        // NOTE: Not needed, since `sizeof` static-assert ensures assignment using the base type is equivalent.
                        //assert(dynamic_cast<impl::${glare_script_file_library}::script_t*>(&self));

                        auto& script_instance = static_cast<impl::${glare_script_file_library}::script_t&>(self);

                        auto fiber = script_instance();
                        
                        while (fiber)
                        {
                            script_instance.on_update();

                            co_yield fiber;
                        }
                    }

                    GLARE_SCRIPT_API
                    ScriptFiber ${glare_script_file_library}(engine::Registry& registry, engine::Entity entity, const engine::MetaEvaluationContext context)
                    {
                        auto script_instance = impl::${glare_script_file_library}::script_t
                        {
                            context,
                            registry,
                            impl::${glare_script_file_library}::script_t::script_id,
                            entity
                        };
                        
                        auto fiber = script_instance();
                        
                        while (fiber)
                        {
                            script_instance.on_update();

                            co_yield fiber;
                        }
                    }
                    
                    template <>
                    ScriptFiber script<impl::${glare_script_file_library}::script_t::script_id>(engine::Script& self, engine::Registry& registry, engine::Entity entity, const engine::MetaEvaluationContext& context)
                    {
                        return ${glare_script_file_library}(self, registry, entity, context);
                    }

                    template <>
                    ScriptFiber script<impl::${glare_script_file_library}::script_t::script_id>(engine::Registry& registry, engine::Entity entity, const engine::MetaEvaluationContext& context)
                    {
                        return ${glare_script_file_library}(registry, entity, context);
                    }

                    template <>
                    ScriptFiber script<impl::${glare_script_file_library}::script_t::script_no_base_path_id>(engine::Script& self, engine::Registry& registry, engine::Entity entity, const engine::MetaEvaluationContext& context)
                    {
                        return ${glare_script_file_library}(self, registry, entity, context);
                    }

                    template <>
                    ScriptFiber script<impl::${glare_script_file_library}::script_t::script_no_base_path_id>(engine::Registry& registry, engine::Entity entity, const engine::MetaEvaluationContext& context)
                    {
                        return ${glare_script_file_library}(registry, entity, context);
                    }
                }
            ")

            set(consume_script_impl_hpp
            "
                #include <script/api.hpp>
                #include <script/script_static_decl.hpp>

                #include <engine/types.hpp>
                #include <engine/entity/script_fiber.hpp>
                #include <engine/meta/hash.hpp>
                
                namespace engine
                {
                    class Script;
                }

                struct GLARE_SCRIPT_API glare_script_impl_reflect_${glare_script_file_library}_t
                {
                    glare_script_impl_reflect_${glare_script_file_library}_t();
                };

                namespace glare
                {
                    extern template
                    ScriptFiber script<engine::hash(\"${glare_script_file_path}\")>(engine::Script& self, engine::Registry& registry, engine::Entity entity, const engine::MetaEvaluationContext& context);

                    extern template
                    ScriptFiber script<engine::hash(\"${glare_script_file_path}\")>(engine::Registry& registry, engine::Entity entity, const engine::MetaEvaluationContext& context);

                    extern template
                    ScriptFiber script<engine::hash(\"engine/${glare_script_file_path}\")>(engine::Script& self, engine::Registry& registry, engine::Entity entity, const engine::MetaEvaluationContext& context);

                    extern template
                    ScriptFiber script<engine::hash(\"engine/${glare_script_file_path}\")>(engine::Registry& registry, engine::Entity entity, const engine::MetaEvaluationContext& context);

                    GLARE_SCRIPT_API
                    ScriptFiber ${glare_script_file_library}(engine::Script& self, engine::Registry& registry, engine::Entity entity, const engine::MetaEvaluationContext context);

                    GLARE_SCRIPT_API
                    ScriptFiber ${glare_script_file_library}(engine::Registry& registry, engine::Entity entity, const engine::MetaEvaluationContext context);

                    namespace impl
                    {
                        namespace ${glare_script_file_library}
                        {
                            class script_t;

                            using ${glare_script_name} = script_t;
                        }
                    }

                    using ${glare_script_name} = impl::${glare_script_file_library}::${glare_script_name};
                }

                extern \"C\" const struct GLARE_SCRIPT_API glare_script_impl_reflect_${glare_script_file_library}_t glare_script_impl_reflect_${glare_script_file_library};

                // NOTE: 32-bit extern-C symbols have slightly different symbol names on MSVC. (leading '_')
                #ifdef GLARE_SCRIPT_EXPORT_SYMBOLS
                    #if defined(_WIN64)
                        __pragma(comment (linker, \"/export:glare_script_impl_reflect_${glare_script_file_library}\"))
                    #else if defined(_WIN32)
                        __pragma(comment (linker, \"/export:_glare_script_impl_reflect_${glare_script_file_library}\"))
                    #endif
                #else
                    #if defined(_WIN64)
                        __pragma(comment (linker, \"/include:glare_script_impl_reflect_${glare_script_file_library}\"))
                    #else if defined(_WIN32)
                        __pragma(comment (linker, \"/include:_glare_script_impl_reflect_${glare_script_file_library}\"))
                    #endif
                #endif
            ")

            set(consume_script_impl_cpp
            "
                #pragma message(\"Building generated code for consumption of script: '${glare_script_local_file_no_ext}'...\")
                
                #include <engine/script.hpp>
                #include <engine/reflection/script.hpp>

                #include <entt/meta/meta.hpp>

                // Debugging related:
                //#include <iostream>
                
                GLARE_SCRIPT_API glare_script_impl_reflect_${glare_script_file_library}_t::glare_script_impl_reflect_${glare_script_file_library}_t()
                {
                    using namespace engine::literals;

                    static constexpr auto script_name_id                           = \"${glare_script_name}\"_hs;
                    
                    static constexpr auto script_local_name_id                     = \"${glare_script_local_file_no_ext}\"_hs;

                    static constexpr auto script_file_id                           = \"engine/${glare_script_file_path}\"_hs;
                    static constexpr auto script_file_no_extension_id              = \"engine/${glare_script_file_path_without_extension}\"_hs;
                    
                    static constexpr auto script_file_no_base_path_id              = \"${glare_script_file_path}\"_hs;
                    static constexpr auto script_file_no_base_path_no_extension_id = \"${glare_script_file_path_without_extension}\"_hs;

                    static constexpr auto script_impl_inplace    = static_cast<glare::ScriptFiber(*)(engine::Script&, engine::Registry&, engine::Entity, const engine::MetaEvaluationContext&)>(&glare::script<script_file_id>);
                    static constexpr auto script_impl_standalone = static_cast<glare::ScriptFiber(*)(engine::Registry&, engine::Entity, const engine::MetaEvaluationContext&)>(&glare::script<script_file_id>);

                    static constexpr auto script_impl_no_base_path_inplace    = static_cast<glare::ScriptFiber(*)(engine::Script&, engine::Registry&, engine::Entity, const engine::MetaEvaluationContext&)>(&glare::script<script_file_no_base_path_id>);
                    static constexpr auto script_impl_no_base_path_standalone = static_cast<glare::ScriptFiber(*)(engine::Registry&, engine::Entity, const engine::MetaEvaluationContext&)>(&glare::script<script_file_no_base_path_id>);

                    auto reflect_type = [&](auto& type)
                    {
                        type = type
                            .func<script_impl_inplace>(script_file_id)
                            .func<script_impl_inplace>(script_file_no_extension_id)
                            .func<script_impl_inplace>(script_name_id)

                            .func<script_impl_standalone>(script_file_id)
                            .func<script_impl_standalone>(script_file_no_extension_id)
                            .func<script_impl_standalone>(script_name_id)

                            // NOTE: May be ambiguous due to name conflicts.
                            .func<script_impl_standalone>(script_local_name_id)

                            .func<script_impl_no_base_path_inplace>(script_file_no_base_path_id)
                            .func<script_impl_no_base_path_inplace>(script_file_no_base_path_no_extension_id)

                            .func<script_impl_no_base_path_standalone>(script_file_no_base_path_id)
                            .func<script_impl_no_base_path_standalone>(script_file_no_base_path_no_extension_id)
                        ;

                        return type;
                    };

				    auto shared_type = entt::meta<engine::ScriptNamespace>();

                    reflect_type(shared_type);

                    auto script_type = entt::meta<glare::impl::${glare_script_file_library}::script_t>()
                        .base<engine::Script>()
                        .type(script_file_no_extension_id) // script_name_id

                        .func<script_impl_inplace>(\"main\"_hs)
                        .func<script_impl_standalone>(\"main\"_hs)

                        .prop(\"script_cpp\"_hs)
                        .prop(\"script\"_hs)
                    ;

                    reflect_type(script_type);

                    #if GLARE_SCRIPT_IS_COROUTINE
                        script_type = script_type.prop("coroutine"_hs);
                    #endif // GLARE_SCRIPT_IS_COROUTINE

                    // Debugging related:
				    //std::cout << \"Reflecting script file: '${glare_script_file_path}'\" << '\\n';
                }
                
                const struct GLARE_SCRIPT_API glare_script_impl_reflect_${glare_script_file_library}_t glare_script_impl_reflect_${glare_script_file_library} {};
            ")

            if (GLARE_SCRIPT_PRECOMPILED_GENERATE_HEADER_FILES)
                file(WRITE ${glare_script_output_hpp_file} "${script_file_header_content}")

                string(PREPEND before_script "#include <script/${glare_script_output_hpp_file}>\n\n")
            else()
                STRING(PREPEND before_script "${script_file_header_content}" "\n\n")
            endif()

            STRING(PREPEND before_script "#pragma message(\"Precompiling script file: '${glare_script_local_file_no_ext}' (${glare_script_file_path})\")\n")

            if (GLARE_SCRIPT_PRECOMPILED_GENERATE_CONSUMPTION_FILES)
                string(PREPEND consume_script_impl_hpp "#pragma once\n\n")

                file(WRITE ${glare_consume_script_output_hpp_file}
                    "${consume_script_impl_hpp}"
                )
                
                if (GLARE_SCRIPT_PRECOMPILED_GENERATE_HEADER_FILES)
                    string(PREPEND consume_script_impl_cpp "#include <script/${glare_script_output_hpp_file}>\n\n")
                endif()

                string(PREPEND consume_script_impl_cpp "#include <script/${glare_consume_script_output_hpp_file}>\n\n")

                file(WRITE ${glare_consume_script_output_cpp_file}
                    "${consume_script_impl_cpp}"
                )

                target_sources(
                    glare_script

                    PRIVATE

                    ${glare_consume_script_output_cpp_file}
                )

                # Append to the shared 'all scripts' header.
                string(
                    APPEND glare_consume_all_scripts_hpp_content
                    "#include <script/${glare_consume_script_output_hpp_file}>\n"
                )
            else()
                string(APPEND after_script "\n\n" "${consume_script_impl_hpp}" "\n\n" "${consume_script_impl_cpp}")
            endif()

            file(WRITE ${glare_script_output_cpp_file} "${before_script}" "${script_file_content}" "${after_script}")

            target_sources(
                glare_script

                PRIVATE

                ${glare_script_output_cpp_file}
            )

            # Ensure symbol for script reflection is included on MSVC:
            if (WIN32)
                if (CMAKE_SIZEOF_VOID_P EQUAL 8) # 64-bit Windows
                    target_link_options(glare_script PUBLIC "/include:glare_script_impl_reflect_${glare_script_file_library}")
                else() # 32-bit Windows
                    # 32-bit x86 symbols have leading '_' character.
                    target_link_options(glare_script PUBLIC "/include:_glare_script_impl_reflect_${glare_script_file_library}")
                endif()
            endif()
        endif()
    endforeach()

    if (GLARE_SCRIPT_PRECOMPILED_GENERATE_CONSUMPTION_FILES)
        file(WRITE ${glare_consume_all_scripts_hpp_output_file} "${glare_consume_all_scripts_hpp_content}")
    endif()
endif()

target_compile_definitions(glare_script PRIVATE GLARE_SCRIPT_EXPORT_SYMBOLS=1)

target_include_directories(
    glare_script
    ${warning_guard}
    PRIVATE
    "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/glare/src>"
)

if (WIN32)
    target_link_libraries(
        glare_script

        PRIVATE
        
        "version.lib" # Required for LLVM/Clang.
    )
endif()

if(GLARE_CLING_USE_THIRDPARTY)
    #find_package(cling REQUIRED)
    #find_package(clingInterpreter REQUIRED)

    target_link_libraries(
        glare_script

        PRIVATE
        
        #Cling::Cling
        #Cling::clingMetaProcessor
        Cling::ClingInterpreter
        Cling::clingUtils

        # Clang libraries used by Cling:
        Cling::clangFrontend
        Cling::clangSerialization
        Cling::clangParse
        Cling::clangSema
        Cling::clangAnalysis
        Cling::clangEdit
        Cling::clangLex
        Cling::clangDriver
        Cling::clangCodeGen
        Cling::clangBasic
        Cling::clangAST

        Cling::clangASTMatchers
        Cling::clangSupport

        # LLVM libraries used by Cling:

        #"${LLVM_AVAILABLE_LIBS}" # <-- Everything output by llvm-config on build/generate. (Many more libraries than is necessary)

        LLVMBitWriter
        LLVMMCJIT
        LLVMOrcJIT
        LLVMOption
        LLVMipo
        LLVMProfileData
        LLVMInstrumentation
        LLVMObjCARCOpts

        LLVMX86CodeGen
        LLVMX86AsmParser        
        #LLVMX86Disassembler
        #LLVMX86TargetMCA
        LLVMX86Desc
        LLVMX86Info

        LLVMNVPTXCodeGen
        LLVMNVPTXDesc
        LLVMNVPTXInfo

        LLVMFrontendHLSL
        LLVMCoverage
        LLVMLTO
    )

    #find_package(LLVM CONFIG REQUIRED)

    #message(LLVM_AVAILABLE_LIBS="${LLVM_AVAILABLE_LIBS}")
endif()