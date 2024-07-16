function(glare_gather_shared_sources_ex shared_sources_out shared_source_file_name base_path submodule_paths)
    #message("Gathering shared sources named '${shared_source_file_name}' from \"${base_path}\"...")

    file(
        GLOB_RECURSE sources_out
        CONFIGURE_DEPENDS
        "${base_path}/*/${shared_source_file_name}"
    )

    if(NOT ${submodule_paths} STREQUAL "")
        foreach(submodule_path ${submodule_paths})
            #message("Adding submodule files to shared sources: \"${submodule_path}\"...")
            
            set(submodule_source_files "")

            file(
                GLOB_RECURSE submodule_source_files
                CONFIGURE_DEPENDS
                ${submodule_path}/*.cpp
            )

            foreach(submodule_source_file_path ${submodule_source_files})
                #message(submodule_source_file_path=${submodule_source_file_path})

                # Remove submodule files from the list of sources, if present.
                list(REMOVE_ITEM sources_out "${submodule_source_file_path}")

                # Add the submodule files back to the end of the list, ensuring that all other files are included first.
                list(APPEND sources_out "${submodule_source_file_path}")
            endforeach()
        endforeach()

        #message("Ensuring submodules' '${shared_source_file_name}' files are added last...")

        # Manually move the shared source files in `submodule_paths` to the end of the list, in linear order:
        foreach(submodule_path ${submodule_paths})
            set(submodule_primary_shared_source_file "${submodule_path}/${shared_source_file_name}")

            #message(submodule_primary_shared_source_file=${submodule_primary_shared_source_file})

            if (EXISTS "${submodule_primary_shared_source_file}")
                #message("Moving \"${submodule_primary_shared_source_file}\" to the end of the list.")

                list(REMOVE_ITEM sources_out ${submodule_primary_shared_source_file})
                list(APPEND sources_out "${submodule_primary_shared_source_file}")
            endif()
        endforeach()
    endif()

    #message("Shared source files gathered:")

    #foreach(source_file_path ${sources_out})
    #    message(${source_file_path})
    #endforeach()

    set(${shared_sources_out} "${sources_out}" PARENT_SCOPE)
endfunction()

function(glare_gather_shared_sources shared_sources_out shared_source_file_name base_path)
    set(submodule_paths "")
    set(sources_out "")

    glare_gather_shared_sources(sources_out ${shared_source_file_name} ${base_path} ${submodule_paths})

    set(${shared_sources_out} "${sources_out}" PARENT_SCOPE)
endfunction()