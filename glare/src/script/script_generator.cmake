function(glare_generate_cpp_script_bindings script_target script_directory script_reference_base_directory script_reference_base_directory_alias include_global_header)
    message("Generating scripts for: ${script_target} @ \"${script_directory}\"...")

    target_compile_definitions(${script_target} PRIVATE GLARE_SCRIPT_INCLUDE_GLOBAL_HEADER=${include_global_header})

    #target_compile_definitions(${script_target} PUBLIC GLARE_SCRIPT_PRECOMPILED=${GLARE_SCRIPT_PRECOMPILED})
    #target_compile_definitions(${script_target} PUBLIC GLARE_SCRIPT_CLING_ENABLED=${GLARE_SCRIPT_CLING_ENABLED})

    if(GLARE_SCRIPT_CLING_ENABLED)
        target_compile_definitions(${script_target} PRIVATE GLARE_SCRIPT_CLING=1)
        #target_compile_definitions(${script_target} PUBLIC GLARE_SCRIPT_PRECOMPILED=0)

        #set_target_properties(${script_target} PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS 1)
    endif()

    #message(GLARE_SCRIPT_PRECOMPILED=${GLARE_SCRIPT_PRECOMPILED})

    if(GLARE_SCRIPT_PRECOMPILED)
        #set_target_properties(${script_target} PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS 1)
        set_target_properties(${script_target} PROPERTIES ENABLE_EXPORTS 1)

        if(GLARE_SCRIPT_PRECOMPILED_UNITY)
            set_target_properties(
                ${script_target}
                PROPERTIES
                
                UNITY_BUILD ON

                #UNITY_BUILD_MODE BATCH
                UNITY_BUILD_MODE GROUP
            )
        endif()
    
        target_link_libraries(
            ${script_target}
            
            PRIVATE
            
            glare_core
            glare_engine
        )

        target_compile_definitions(${script_target} PRIVATE GLARE_SCRIPT_EXPORT_SYMBOLS=1)

        # Enumerate script source files:
        file(
            GLOB_RECURSE script_sources
            RELATIVE ${script_reference_base_directory}

            CONFIGURE_DEPENDS
        
            ${script_directory}/*.cpp

            #${script_directory}/*.cxx
            #${script_directory}/*.C
            #${script_directory}/*.ixx
            #${script_directory}/*.cppm

            ##${script_directory}/*.h
            ##${script_directory}/*.hpp

            #${script_directory}/*.hxx
            #${script_directory}/*.H
        )

        #message(script_sources="${script_sources}")

        if (GLARE_SCRIPT_PRECOMPILED_GENERATE_CONSUMPTION_FILES)
            set(glare_consume_all_scripts_hpp_output_file "generated/script/latest/consume_scripts.hpp")

            set(glare_consume_all_scripts_hpp_content
            "
                #pragma once

            ")
        endif()

        #if(NOT script_sources)
        #    message("No script sources found in: \"${script_directory}\"")
        #endif()

        # Currently one file per generated library. (May later group by folder, etc.)
        foreach(glare_script_file_path ${script_sources})
            if (false) # (${glare_script_file_path} MATCHES ".+\.(h|hpp|hxx|H)$")
                #message("Skipping code generation for header file: \"${glare_script_file_path}\"")
            else()
                string(REGEX REPLACE "\\.[^.]*$" "" glare_script_file_path_without_extension ${glare_script_file_path})
                string(REGEX REPLACE "[^a-zA-Z\_]+" "_" glare_script_name "${glare_script_file_path_without_extension}")

                #cmake_path(GET ${glare_script_file_path} FILENAME glare_script_local_file)
                get_filename_component(glare_script_local_file "${glare_script_file_path}" NAME)

                get_filename_component(glare_script_local_directory "${glare_script_file_path}" DIRECTORY)

                #message(glare_script_local_file=${glare_script_local_file})
                #message(glare_script_local_directory=${glare_script_local_directory})

                get_filename_component(glare_script_local_file_no_ext "${glare_script_file_path}" NAME_WLE)

                #string(REGEX MATCH "([^\.]+)\..*$" _ ${glare_script_local_file})
                #set(glare_script_local_file_no_ext "${CMAKE_MATCH_1}")

                set(glare_script_file_library "glare_script_${glare_script_name}")

                set(glare_script_output_cpp_file "${CMAKE_CURRENT_BINARY_DIR}/generated/script/latest/${glare_script_file_library}.cpp")
                set(glare_script_output_cpp_file_cached "generated/script/cached/${glare_script_file_library}.cpp")
                set(glare_script_output_hpp_file "${CMAKE_CURRENT_BINARY_DIR}/generated/script/latest/${glare_script_file_library}.hpp")
                set(glare_script_output_hpp_file_cached "generated/script/cached/${glare_script_file_library}.hpp")
                set(glare_consume_script_output_hpp_file "${CMAKE_CURRENT_BINARY_DIR}/generated/script/latest/${glare_script_file_library}_consume.hpp")
                set(glare_consume_script_output_hpp_file_cached "generated/script/cached/${glare_script_file_library}_consume.hpp")
                set(glare_consume_script_output_cpp_file "${CMAKE_CURRENT_BINARY_DIR}/generated/script/latest/${glare_script_file_library}_consume.cpp")
                set(glare_consume_script_output_cpp_file_cached "generated/script/cached/${glare_script_file_library}_consume.cpp")

                message("Generating prebuilt script source file: \"${glare_script_file_path}\" -> \"${glare_script_output_cpp_file}\"")

                set(script_file_content "")

                set(script_file_header_content
                "
                    // Standard includes for `${glare_script_file_library}`:
                    #if GLARE_SCRIPT_INCLUDE_GLOBAL_HEADER
                        #include <script_common.hpp>
                    #endif // GLARE_SCRIPT_INCLUDE_GLOBAL_HEADER

                    #include <script/api.hpp>
                    #include <script/script_static_decl.hpp>
                    #include <script/script_common.hpp>
                    #include <script/code_generation_def.hpp>

                    #include <engine/types.hpp>
                    #include <engine/script/script.hpp>
                    #include <engine/script/awaiters.hpp>
                    #include <engine/script/co_await.hpp>
                    #include <engine/script/code_generation.hpp>
                    #include <engine/script/embedded_script.hpp>

                    //#include <engine/meta/hash.hpp>

                    #include <utility>
                    #include <cassert>

                    #define script_${glare_script_local_file_no_ext} \\
                        glare::impl::${glare_script_file_library}::script_ex_t

                    namespace glare
                    {
                        namespace impl
                        {
                            namespace ${glare_script_file_library}
                            {
                                class script_t;
                                class script_ex_t;

                                using script_ex = script_ex_t;

                                using ${glare_script_name}_ex = script_ex_t;

                                using ${glare_script_name}_t = script_t;
                                using ${glare_script_name} = ${glare_script_name}_t;

                                using ${glare_script_local_file_no_ext}_t = script_t;
                                using ${glare_script_local_file_no_ext} = ${glare_script_local_file_no_ext}_t;
                            }
                        }
                    }
                ")
                
                if (GLARE_SCRIPT_PRECOMPILED_GENERATE_HEADER_FILES)
                    string(PREPEND script_file_header_content "#pragma once\n\n")
                endif()
            
                set(NAME_WLE)

                #string(REGEX MATCH "(.+)\.(cpp|cxx|C)$" _ ${glare_script_file_path})
                #string(REGEX MATCH "(.+)\.(cpp|cxx|C)$" _ ${glare_script_local_file})
                
                #set(glare_script_header_filter_name "${CMAKE_MATCH_1}")

                set(glare_script_header_filter_name "${glare_script_local_file_no_ext}")

                #message(glare_script_header_filter_name=${glare_script_header_filter_name})

                set(script_header_file_names "")

                #list(APPEND script_header_file_names
                #    "${glare_script_header_filter_name}.h"
                #    "${glare_script_header_filter_name}.hpp"
                #    "${glare_script_header_filter_name}.hxx"
                #    "${glare_script_header_filter_name}.H"
                #    "${glare_script_name}.h"
                #    "${glare_script_name}.hpp"
                #    "${glare_script_name}.hxx"
                #    "${glare_script_name}.H"
                #    
                #    "include.hpp"
                #    "includes.hpp"
                #    
                #    "script.hpp"
                #
                #    "common.hpp"
                #    "shared.hpp"
                #    "conditions.hpp"
                #
                #    "util.hpp"
                #
                #    "components.hpp"
                #    "events.hpp"
                #    "reflection.hpp"
                #)

                list(APPEND script_header_file_names
                    ##"*.h"
                    "*.hpp"
                    ##"*.hxx"
                    ##"*.H"
                )
                
                #message(script_header_file_names=${script_header_file_names})

                set(script_header_file_directories "")

                #message(script_directory=${script_directory})
                #message(script_reference_base_directory=${script_reference_base_directory})

                list(APPEND script_header_file_directories
                    "${script_directory}"
                    "${script_directory}/include"
                    "${script_reference_base_directory}/${glare_script_local_directory}"
                    "${script_reference_base_directory}/${glare_script_local_directory}/include"
                    #"${script_reference_base_directory}"
                    #"${script_reference_base_directory}/include"
                )

                list(REMOVE_DUPLICATES script_header_file_directories)

                #message(script_header_file_directories=${script_header_file_directories})

                set(script_header_file_search_paths "")

                foreach(script_search_directory ${script_header_file_directories})
                    foreach(script_header_name ${script_header_file_names})
                        list(APPEND script_header_file_search_paths "${script_search_directory}/${script_header_name}")
                    endforeach()
                endforeach()

                #message(script_header_file_search_paths=${script_header_file_search_paths})
                
                #message("script_header_file_search_paths:")
                #foreach(header_search_path ${script_header_file_search_paths})
                #    message(${header_search_path})
                #endforeach()

                file(
                    GLOB script_header_files
                    #RELATIVE ${script_reference_base_directory}

                    CONFIGURE_DEPENDS
        
                    ${script_header_file_search_paths}
                )

                #message(script_header_files=${script_header_files})
            
                foreach(script_header ${script_header_files})
                    set(script_header_full_path "${script_header}") # "${script_reference_base_directory}/${script_header}"
                    
                    if (GLARE_SCRIPT_EMBED_SOURCE)
                        #message("Found header file for script: \"${script_header}\", embedding directly...")

                        file(STRINGS "${script_header_full_path}" script_file_content_hpp)
                    
                        list(APPEND script_file_header_content "${script_file_content_hpp}" "\n\n")
                    else()
                        #message("Found header file for script: \"${script_header}\", embedding include...")
                
                        string(APPEND script_file_header_content "#include \"${script_header_full_path}\"\n")
                    endif()
                endforeach()

                if (GLARE_SCRIPT_EMBED_SOURCE)
                    # Read entire file up front:
                    file(STRINGS ${script_directory}/${glare_script_file_path} script_file_content_cpp)

                    string(APPEND script_file_content ${script_file_content_cpp})
                else()
                    # Include file instead:
                    string(APPEND script_file_content "#include \"${script_reference_base_directory}/${glare_script_file_path}\"\n")
                endif()

                string(COMPARE NOTEQUAL "${script_file_header_content}" "" script_file_header_content_exists)

                if (script_file_header_content_exists)
                    string(APPEND script_file_header_content "\n\n")
                endif()

                string(APPEND script_file_header_content
                "
                    #undef script_${glare_script_local_file_no_ext}

                    // Declarations for `${glare_script_file_library}`:
                    #define ${glare_script_file_library}_run_decl_params \\
                        GLARE_SCRIPT_ENTRYPOINT_PARAMETER_LIST

                    namespace glare
                    {
                        namespace impl
                        {
                            namespace ${glare_script_file_library}
                            {
                                using namespace glare_script_common;
                                using namespace engine::script;

                                class script_t;
                                class script_ex_t;

                                //GLARE_SCRIPT_API
                                GLARE_GENERATE_SCRIPT_TYPE(\"${glare_script_file_path}\");
                            }
                        }
                    }
                ")

                set(before_script
                "
                    #define _script_no_return_value                              \\
                        glare::impl::${glare_script_file_library}::script_t::operator() \\
                            (${glare_script_file_library}_run_decl_params)
                    
                    //#define _script_on_enter_no_return_value                            \\
                    //    glare::impl::${glare_script_file_library}::script_t::on_enter_impl() \\
                    //        (${glare_script_file_library}_run_decl_params)

                    //#define _script_on_exit_no_return_value                            \\
                    //    glare::impl::${glare_script_file_library}::script_t::on_exit_impl() \\
                    //        (${glare_script_file_library}_run_decl_params)

                    //#define ${glare_script_local_file_no_ext}(...) \\
                    //    glare::ScriptFiber _script_no_return_value

                    #define ${glare_script_local_file_no_ext}(...) \\
                        _script_no_return_value -> glare::ScriptFiber

                    //#define ${glare_script_local_file_no_ext}_on_enter(...) \\
                    //    _script_on_enter_no_return_value -> glare::ScriptFiber

                    //#define ${glare_script_local_file_no_ext}_on_exit(...) \\
                    //    _script_on_exit_no_return_value -> glare::ScriptFiber

                    #define script_${glare_script_local_file_no_ext} \\
                        glare::impl::${glare_script_file_library}::script_ex_t

                    // Too general:
                    //#define main(...) \\
                    //    glare::ScriptFiber _script_no_return_value

                    //#define script(...) \\
                    //    glare::ScriptFiber _script_no_return_value

                    // Macro shorthand:
                    #define SCRIPT_LAMBDA \\
                        GLARE_SCRIPT_LAMBDA_BEGIN

                    #define SCRIPT_LAMBDA_BEGIN \\
                        GLARE_SCRIPT_LAMBDA_BEGIN

                    #define SCRIPT_LAMBDA_END \\
                        GLARE_SCRIPT_LAMBDA_END

                    #define END_SCRIPT_LAMBDA \\
                        GLARE_SCRIPT_LAMBDA_END

                    #define SCRIPT \\
                        GLARE_SCRIPT_BEGIN

                    #define SCRIPT_BEGIN \\
                        GLARE_SCRIPT_BEGIN

                    #define SCRIPT_END \\
                        GLARE_SCRIPT_END

                    #define END_SCRIPT \\
                        GLARE_SCRIPT_END

                ")
                #       { Script body }
                set(after_script
                "
                    
                    // Macro shorthand:
                    #undef SCRIPT_LAMBDA
                    #undef SCRIPT_LAMBDA_BEGIN
                    #undef SCRIPT_LAMBDA_END
                    #undef END_SCRIPT_LAMBDA
                    #undef SCRIPT
                    #undef SCRIPT_BEGIN
                    #undef SCRIPT_END
                    #undef END_SCRIPT
                    
                    //#undef script
                    //#undef main
                    #undef _script_no_return_value

                    #undef ${glare_script_local_file_no_ext}

                    #undef script_${glare_script_local_file_no_ext}

                    GLARE_SCRIPT_DEFINE_BOOTSTRAP_FUNCTIONS(${glare_script_file_library});
                    GLARE_SCRIPT_DEFINE_BOOTSTRAP_BINDINGS(${glare_script_file_library});
                ")

                set(glare_script_symbol_decl "")

                glare_generate_reflection_automation_decl(
                    "glare_script_impl_reflect_${glare_script_file_library}"

                    "GLARE_SCRIPT_API"
                    "GLARE_SCRIPT_EXPORT_SYMBOLS"

                    glare_script_symbol_decl
                )

                set(consume_script_impl_hpp
                "
                    #include <script/api.hpp>
                    #include <script/script_static_decl.hpp>
                    #include <script/code_generation_decl.hpp>

                    #include <engine/types.hpp>
                    #include <engine/script/script_fiber.hpp>
                    #include <engine/meta/hash.hpp>
                
                    namespace engine
                    {
                        class Script;
                    }

                    GLARE_SCRIPT_DECLARE_BOOTSTRAP_FUNCTIONS(${glare_script_file_library});
                    GLARE_SCRIPT_DECLARE_BOOTSTRAP_BINDINGS(\"${glare_script_file_path}\");

                    namespace glare
                    {
                        namespace impl
                        {
                            namespace ${glare_script_file_library}
                            {
                                class script_t;
                                class script_ex_t;

                                using ${glare_script_name}_t = script_t;
                                using ${glare_script_name} = ${glare_script_name}_t;

                                using ${glare_script_local_file_no_ext}_t = script_t;
                                using ${glare_script_local_file_no_ext} = ${glare_script_local_file_no_ext}_t;
                            }
                        }

                        using ${glare_script_name} = impl::${glare_script_file_library}::${glare_script_name};
                    }

                    ${glare_script_symbol_decl}
                ")

                set(glare_script_symbol_def "")

                glare_generate_reflection_automation_def(
                    "glare_script_impl_reflect_${glare_script_file_library}"
                    
                    "GLARE_SCRIPT_API"

                    "Building generated code for consumption of script: '${glare_script_local_file_no_ext}'..."

                    "
                        glare::impl::glare_script_reflect_impl
                        <
                            glare::impl::${glare_script_file_library}::script_t,
                        
                            \"${glare_script_name}\",
                            \"${glare_script_local_file_no_ext}\",
                            \"${glare_script_file_path}\",
                            \"${glare_script_file_path_without_extension}\",
                            \"${script_reference_base_directory_alias}/${glare_script_file_path}\",
                            \"${script_reference_base_directory_alias}/${glare_script_file_path_without_extension}\"
                        >();
                    "

                    glare_script_symbol_def
                )

                set(consume_script_impl_cpp
                "
                    #include <script/reflection_impl.hpp>

                    ${glare_script_symbol_def}
                ")

                if (GLARE_SCRIPT_PRECOMPILED_GENERATE_HEADER_FILES)
                    file(WRITE ${glare_script_output_hpp_file} "${script_file_header_content}")
                    configure_file(${glare_script_output_hpp_file} ${glare_script_output_hpp_file_cached})

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

                    configure_file(${glare_consume_script_output_hpp_file} ${glare_consume_script_output_hpp_file_cached})
                
                    if (GLARE_SCRIPT_PRECOMPILED_GENERATE_HEADER_FILES)
                        string(PREPEND consume_script_impl_cpp "#include <script/${glare_script_output_hpp_file}>\n\n")
                    endif()

                    string(PREPEND consume_script_impl_cpp "#include <script/${glare_consume_script_output_hpp_file}>\n\n")

                    file(WRITE ${glare_consume_script_output_cpp_file}
                        "${consume_script_impl_cpp}"
                    )

                    configure_file(${glare_consume_script_output_cpp_file} ${glare_consume_script_output_cpp_file_cached})

                    target_sources(
                        ${script_target}

                        PRIVATE

                        ${glare_consume_script_output_cpp_file_cached}
                    )

                    # Append to the shared 'all scripts' header.
                    string(
                        APPEND glare_consume_all_scripts_hpp_content
                        "#include <script/${glare_consume_script_output_hpp_file}>\n"
                    )
                else()
                    string(APPEND after_script "\n\n" "${consume_script_impl_hpp}" "\n\n" "${consume_script_impl_cpp}")
                endif()

                #message(CMAKE_CURRENT_BINARY_DIR="${CMAKE_CURRENT_BINARY_DIR}")

                file(WRITE ${glare_script_output_cpp_file} "${before_script}" "${script_file_content}" "${after_script}")
                configure_file(${glare_script_output_cpp_file} ${glare_script_output_cpp_file_cached})

                target_sources(
                    ${script_target}

                    PRIVATE

                    ${glare_script_output_cpp_file_cached}
                )

                glare_generate_reflection_automation_linkage(
                    ${script_target}

                    "glare_script_impl_reflect_${glare_script_file_library}"
                )

                if(GLARE_SCRIPT_PRECOMPILED_UNITY)
                    set(glare_script_unity_group_suffix "")

                    string(REPLACE "/" "_" glare_script_unity_group_suffix "${glare_script_local_directory}")

                    set(glare_script_unity_group "${script_target}_${glare_script_unity_group_suffix}")

                    #message(glare_script_unity_group=${glare_script_unity_group})
                    #message("\"${glare_script_output_cpp_file_cached}\" -> `${glare_script_unity_group}`")

                    set_source_files_properties(
                        ${glare_script_output_cpp_file_cached}
        
                        PROPERTIES

                        UNITY_GROUP "${glare_script_unity_group}"
                    )
                endif()
            endif()
        endforeach()

        if (GLARE_SCRIPT_PRECOMPILED_GENERATE_CONSUMPTION_FILES)
            file(WRITE ${glare_consume_all_scripts_hpp_output_file} "${glare_consume_all_scripts_hpp_content}")
            configure_file(${glare_consume_all_scripts_hpp_output_file} ${glare_consume_all_scripts_hpp_output_file_cached})
        endif()
    endif()

    target_include_directories(
        ${script_target}
        ${warning_guard}
        PRIVATE
        "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/glare/src>"
    )

    if (WIN32)
        target_link_libraries(
            ${script_target}

            PRIVATE
        
            "version.lib" # Required for LLVM/Clang.
        )
    endif()

    if(GLARE_CLING_USE_THIRDPARTY)
        #find_package(cling REQUIRED)
        #find_package(clingInterpreter REQUIRED)

        target_link_libraries(
            ${script_target}

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
endfunction()