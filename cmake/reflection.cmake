function(glare_add_reflection_sources_to_target reflection_target base_path reflection_submodule_paths reflection_unity_group)
    message("Gathering reflection source files for '${reflection_target}' from \"${base_path}\"...")
    
    if(GLARE_REFLECTION_USE_UNITY_BUILD)
        if(${reflection_unity_group} STREQUAL "")
            set_target_properties(
                ${reflection_target}
    
                PROPERTIES

                UNITY_BUILD ON
                UNITY_BUILD_MODE BATCH
            )
        else()
            set_target_properties(
                ${reflection_target}
    
                PROPERTIES

                UNITY_BUILD ON
                UNITY_BUILD_MODE GROUP
            )
        endif()
    endif()

    set(reflection_sources "")

    glare_gather_shared_sources_ex(reflection_sources "reflection.cpp" ${base_path} ${reflection_submodule_paths})

    #message("reflection_sources:")
    
    #foreach(reflection_source_file ${reflection_sources})
    #    message("${reflection_source_file}")
    #endforeach()

    target_sources(
        ${reflection_target}
        
        PRIVATE

        ${reflection_sources}
    )

    if(GLARE_REFLECTION_USE_UNITY_BUILD)
        if(NOT ${reflection_unity_group} STREQUAL "")
            set_source_files_properties(
                ${reflection_sources}
        
                PROPERTIES

                UNITY_GROUP "${reflection_unity_group}"
            )
        endif()
    endif()
endfunction()

function(glare_generate_reflection_automation_decl_type symbol_name reflection_symbol_macro type_content_out)
    set(type_content
    "
        struct ${reflection_symbol_macro} ${symbol_name}_t
        {
            ${symbol_name}_t();
        };
    ")

    #message("glare_generate_reflection_automation_decl_type -> type_content:")
    #message("${type_content}")

    set(${type_content_out} "${type_content}" PARENT_SCOPE)
endfunction()

function(glare_generate_reflection_automation_forward_decl_type symbol_name reflection_symbol_macro type_content_out)
    set(type_content
    "
        struct ${reflection_symbol_macro} ${symbol_name}_t;
    ")

    set(${type_content_out} "${type_content}" PARENT_SCOPE)
endfunction()

function(glare_generate_symbol_linkage_directive symbol_name symbol_export_macro symbol_linkage_content_out)
    set(symbol_linkage_content
    "
        // NOTE: 32-bit extern-C symbols have slightly different symbol names on MSVC. (leading '_')
        #ifdef ${symbol_export_macro}
            #if defined(_WIN64)
                __pragma(comment (linker, \"/export:${symbol_name}\"))
            #else if defined(_WIN32)
                __pragma(comment (linker, \"/export:_${symbol_name}\"))
            #endif
        #else
            #if defined(_WIN64)
                __pragma(comment (linker, \"/include:${symbol_name}\"))
            #else if defined(_WIN32)
                __pragma(comment (linker, \"/include:_${symbol_name}\"))
            #endif
        #endif
    ")

    set(${symbol_linkage_content_out} "${symbol_linkage_content}" PARENT_SCOPE)
endfunction()

function(glare_generate_reflection_automation_decl symbol_name reflection_symbol_macro reflection_symbol_export_macro automation_decl_out)
    set(type_content "")

    glare_generate_reflection_automation_decl_type("${symbol_name}" "${reflection_symbol_macro}" type_content)
    
    #message("type_content:")
    #message("${type_content}")

    set(symbol_var_decl
    "
        extern \"C\" const struct ${reflection_symbol_macro} ${symbol_name}_t ${symbol_name};
    ")

    set(symbol_linkage_content "")

    glare_generate_symbol_linkage_directive("${symbol_name}" "${reflection_symbol_macro}" symbol_linkage_content)

    set(automation_decl
    "
        ${type_content}

        ${symbol_var_decl}

        ${symbol_linkage_content}
    ")

    set(${automation_decl_out} "${automation_decl}" PARENT_SCOPE)
endfunction()

function(glare_generate_reflection_automation_def_type symbol_name reflection_symbol_macro function_content type_content_out)
    set(type_content
    "
        ${reflection_symbol_macro} ${symbol_name}_t::${symbol_name}_t()
        {
            ${function_content}
        }
    ")

    set(${type_content_out} "${type_content}" PARENT_SCOPE)
endfunction()

function(glare_generate_reflection_automation_def symbol_name reflection_symbol_macro build_message function_content automation_def_out)
    set(type_content "")

    glare_generate_reflection_automation_def_type("${symbol_name}" "${reflection_symbol_macro}" "${function_content}" type_content)

    set(automation_def
    "
        #pragma message(\"${build_message}\")
                
        ${type_content}
        
        const struct ${reflection_symbol_macro} ${symbol_name}_t ${symbol_name} {};
    ")

    set(${automation_def_out} "${automation_def}" PARENT_SCOPE)
endfunction()

function(glare_generate_reflection_automation_linkage reflection_target symbol_name)
    # Ensure symbol for script reflection is included on MSVC:
    if (WIN32)
        if (CMAKE_SIZEOF_VOID_P EQUAL 8) # 64-bit Windows
            target_link_options(${reflection_target} PUBLIC "/include:${symbol_name}")
        else() # 32-bit Windows
            # 32-bit x86 symbols have leading '_' character.
            target_link_options(${reflection_target} PUBLIC "/include:_${symbol_name}")
        endif()
    endif()
endfunction()