function(glare_group_sources_for_unity_build_ex base_path unity_filters unity_group_name unity_file_ignore_list recursive split_into_subgroups)
    set(unity_sources_unfiltered "")

    if (recursive)
        file(
            GLOB_RECURSE unity_sources_unfiltered
            RELATIVE "${base_path}"

            ${unity_filters}
        )
    else()
        file(
            GLOB unity_sources_unfiltered
            RELATIVE "${base_path}"

            ${unity_filters}
        )
    endif()

    set(unity_sources "")

    glare_apply_ignore_list("${unity_sources_unfiltered}" "${unity_file_ignore_list}" unity_sources)

    if(unity_sources)
        list(LENGTH unity_sources number_of_unity_sources)

        #message(number_of_unity_sources=${number_of_unity_sources})

        if(${number_of_unity_sources} GREATER 1)
            #message("Adding sources to unity build group `${unity_group_name}`:")

            #message("`${unity_group_name}`:")

            if(split_into_subgroups)
                foreach(unity_source_file ${unity_sources})
                    set(unity_source_file_directory_path "")

                    get_filename_component(unity_source_file_directory_path "${base_path}/${unity_source_file}" DIRECTORY)

                    #message("unity_source_file=${unity_source_file}")

                    #message("unity_source_file_directory_path:")
                    #message("${unity_source_file_directory_path}")

                    set(unity_source_file_relative_path "")

                    cmake_path(RELATIVE_PATH unity_source_file_directory_path BASE_DIRECTORY "${base_path}")

                    #message("unity_source_file_directory_path (updated):")
                    #message("${unity_source_file_directory_path}")

                    set(unity_source_subgroup "")

                    string(REPLACE "/" "_" unity_source_subgroup_suffix "${unity_source_file_directory_path}")

                    #message(unity_source_subgroup_suffix=${unity_source_subgroup_suffix})

                    set(unity_source_subgroup "${unity_group_name}_${unity_source_subgroup_suffix}")

                    #message(unity_source_subgroup=${unity_source_subgroup})

                    #message("\"${unity_source_file}\" -> `${unity_source_subgroup}`")

                    set_source_files_properties(
                        ${unity_source_file}
        
                        PROPERTIES

                        UNITY_GROUP "${unity_source_subgroup}"
                    )
                endforeach()
            else()
                #foreach(unity_source_file ${unity_sources})
                #    message("\"${unity_source_file}\" -> `${unity_group_name}`")
                #endforeach()
                
                set_source_files_properties(
                    ${unity_sources}
        
                    PROPERTIES

                    UNITY_GROUP "${unity_group_name}"
                )
            endif()
        else()
            #message("Only on file found for `${unity_group_name}`: ${unity_sources}")
            #message("Skipping unity build association.")
        endif()
    else()
        #message("No source files found for `${unity_group_name}`")
    endif()

    #message("")
endfunction()

function(glare_group_sources_for_unity_build base_path submodule_name unity_group_name submodule_file_ignore_list split_into_subgroups)
    if(${unity_group_name} STREQUAL "")
        set(unity_group_name "${submodule_name}")
    endif()

    set(submodule_unity_path "${base_path}/${submodule_name}")
    #set(submodule_unity_path "${submodule_name}")

    #message("Tagging source files in submodule `${submodule_name}` with unity group `${unity_group_name}` from \"${base_path}\"...")
    #message("")
    #message("`${submodule_name}` @ \"${base_path}\"")
    #message("")

    set(
        submodule_unity_filters
        
        "${submodule_unity_path}/*.cpp"
        "${submodule_unity_path}/*/*.cpp"

        #"${submodule_unity_path}/*.cxx"
        #"${submodule_unity_path}/*.C"
        #"${submodule_unity_path}/*.ixx"
    )

    glare_group_sources_for_unity_build_ex("${base_path}" "${submodule_unity_filters}" "${unity_group_name}" "${submodule_file_ignore_list}" true "${split_into_subgroups}")
endfunction()

function(glare_group_submodule_sources_for_unity_build base_path submodule_group_prefix submodule_ignore_list submodule_file_ignore_list split_into_subgroups)
    set(submodule_directories_unfiltered "")
    
    file(
        GLOB
        submodule_directories_unfiltered
        LIST_DIRECTORIES true
        RELATIVE "${base_path}"
        "${base_path}/*"
    )

    #message(submodule_directories_unfiltered=${submodule_directories_unfiltered})

    #message("submodule_directories_unfiltered:")
    #
    #foreach(submodule_directory_entry ${submodule_directories_unfiltered})
    #    message(${submodule_directory_entry})
    #endforeach()

    set(submodule_directories "")

    glare_apply_ignore_list("${submodule_directories_unfiltered}" "${submodule_ignore_list}" submodule_directories)

    #message(submodule_directories=${submodule_directories})
    
    #message("submodule_directories:")
    #
    #foreach(submodule_directory_entry ${submodule_directories})
    #    message(${submodule_directory_entry})
    #endforeach()

    set(submodule_parent_path "${base_path}")
    #set(submodule_parent_path "")

    foreach(submodule_directory_entry ${submodule_directories})
        set(module_directory_entry_path "${base_path}/${submodule_directory_entry}")

        if(IS_DIRECTORY "${module_directory_entry_path}")
            if(${split_into_subgroups})
                set(submodule_group "${submodule_group_prefix}")
            else()
                set(submodule_group "${submodule_group_prefix}${submodule_directory_entry}")
            endif()

            glare_group_sources_for_unity_build("${submodule_parent_path}" "${submodule_directory_entry}" "${submodule_group}" "${submodule_file_ignore_list}" "${split_into_subgroups}")
        endif()
    endforeach()
endfunction()

function(glare_configure_unity_build_for_module_ex module_target module_name base_path unity_submodule_ignore_list unity_file_ignore_list aggregate_entire_module split_into_subgroups)
    set_target_properties(
        ${module_target}
        
        PROPERTIES

        UNITY_BUILD ON
        UNITY_BUILD_MODE GROUP
    )

    set(local_file_filters
        "*.cpp"

        #"*.C"
        #"*.cxx"
        #"*.ixx"
    )

    if(aggregate_entire_module)
        set(consolidated_unity_ignore_list
            ${unity_file_ignore_list}
            ${unity_submodule_ignore_list}
        )

        LIST(REMOVE_DUPLICATES consolidated_unity_ignore_list)

        glare_group_sources_for_unity_build_ex("${base_path}" "${local_file_filters}" "${module_name}" "${consolidated_unity_ignore_list}" true false)
    else()
        if(${split_into_subgroups})
            set(submodule_group_prefix "${module_name}")
        else()
            set(submodule_group_prefix "${module_name}_")
        endif()

        glare_group_submodule_sources_for_unity_build("${base_path}" "${submodule_group_prefix}" "${unity_submodule_ignore_list}" "${unity_file_ignore_list}" "${split_into_subgroups}")

        glare_group_sources_for_unity_build_ex("${base_path}" "${local_file_filters}" "${module_name}" "${unity_file_ignore_list}" false false)
    endif()
endfunction()

function (glare_configure_unity_build_for_module module_target base_path aggregate_entire_module split_into_subgroups)
    set(unity_file_ignore_list "")

    glare_ignore_list("unity_build.ignore" unity_file_ignore_list)

    if(aggregate_entire_module)
        set(unity_submodule_ignore_list "")
    else()
        set(unity_submodule_ignore_list ${unity_file_ignore_list})
    endif()

    glare_configure_unity_build_for_module_ex("${module_target}" "${module_target}" "${base_path}" "${unity_submodule_ignore_list}" "${unity_file_ignore_list}" "${aggregate_entire_module}" "${split_into_subgroups}")
endfunction()