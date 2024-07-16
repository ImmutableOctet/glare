function(glare_gather_files_basic base_path file_filters files_out)
    set(files_requested "")

    set(base_relative_file_filters "")

    foreach(file_filter ${file_filters})
        list(APPEND base_relative_file_filters "${base_path}/${file_filter}")
        list(APPEND base_relative_file_filters "${base_path}/*/${file_filter}")
    endforeach()

    file(
        GLOB_RECURSE files_requested
        #CONFIGURE_DEPENDS
        RELATIVE ${base_path}
        
        ${base_relative_file_filters}
    )

    #message(files_requested=${files_requested})

    set(${files_out} "${files_requested}" PARENT_SCOPE)
endfunction()

function(glare_gather_files_advanced base_path general_file_filters detailed_file_filters files_out)
    set(base_relative_file_filters "")

    #message(general_file_filters=${general_file_filters})

    foreach(file_filter ${general_file_filters})
        list(APPEND base_relative_file_filters "${base_path}/${file_filter}")
        list(APPEND base_relative_file_filters "${base_path}/*/${file_filter}")
    endforeach()
    
    #message("base_relative_file_filters:")
    #foreach(file_filter ${base_relative_file_filters})
    #    message(${file_filter})
    #endforeach()

    set(files_requested "")
    
    #set(nested_directories "")

    #foreach(base_relative_filter ${base_relative_file_filters})
    #    message(base_relative_filter=${base_relative_filter})
    #    file(
    #        GLOB_RECURSE nested_directories
    #        LIST_DIRECTORIES true
    #
    #        ${base_relative_filter}
    #    )
    #endforeach()

    #list(REMOVE_DUPLICATES nested_directories)

    file(
        GLOB_RECURSE nested_directories
        LIST_DIRECTORIES true
        #RELATIVE ${base_path}

        ${base_relative_file_filters}
    )

    #message(nested_directories=${nested_directories})

    foreach(nested_path ${nested_directories})
        if(IS_DIRECTORY ${nested_path}) # "${base_path}/${nested_path}"
            #message(nested_path=${nested_path})

            set(nested_relative_file_filters "")

            foreach(detailed_filter ${detailed_file_filters})
                set(nested_filter "${nested_path}/${detailed_filter}")

                if(${nested_filter} MATCHES "\/$")
                    foreach(general_filter ${general_file_filters})
                        set(resolved_filter "${nested_filter}${general_filter}")

                        list(APPEND nested_relative_file_filters "${resolved_filter}")
                    endforeach()
                else()
                    list(APPEND nested_relative_file_filters "${nested_filter}")
                endif()
            endforeach()

            #message(nested_relative_file_filters=${nested_relative_file_filters})

            file(
                GLOB nested_files_found
                LIST_DIRECTORIES false

                RELATIVE ${base_path}

                ${nested_relative_file_filters}
            )

            #message(nested_files_found=${nested_files_found})

            foreach(nested_file ${nested_files_found})
                #message(nested_file=${nested_file})
                
                list(APPEND files_requested "${nested_file}")

                #message("Nested file added: ${nested_file}")

                #foreach(general_filter ${general_file_filters})
                #    if(${nested_file} MATCHES "${general_filter}")
                #        list(APPEND files_requested "${nested_file}")
                #
                #        message("Nested file added: ${nested_file}")
                #
                #        break()
                #    endif()
                #endforeach()
            endforeach()
        endif()
    endforeach()

    #message(files_requested=${files_requested})

    set(${files_out} "${files_requested}" PARENT_SCOPE)
endfunction()

function(glare_gather_headers_basic base_path file_name_filters files_out)
    set(header_file_extension_list
        "hpp"

        ##"h"
        ##"hxx"

        #"ixx"
    )

    set(header_filters "")

    foreach(header_suffix ${header_file_extension_list})
        foreach(file_filter ${file_name_filters})
            list(APPEND header_filters "${file_filter}.${header_suffix}")
        endforeach()
    endforeach()

    set(files_found "")

    glare_gather_files_basic("${base_path}" "${header_filters}" files_found)

    set(${files_out} "${files_found}" PARENT_SCOPE)
endfunction()

# Multi-pass implementation (supports recursion into folders sharing a common name, but arbitrary depth):
function(glare_gather_headers_advanced base_path header_filters headers_out)
    set(header_file_general_filters
        "*.h"
        "*.hpp"
        "*.hxx"
    )

    set(headers_found "")

    glare_gather_files_advanced(
        "${base_path}"
        "${header_file_general_filters}"
        "${header_filters}"

        headers_found
    )

    set(${headers_out} "${headers_found}" PARENT_SCOPE)
endfunction()

function(glare_gather_component_headers component_headers_out base_path ignored_paths)
    set(headers_unfiltered "")

    glare_gather_headers_basic("${base_path}" "*_component" headers_unfiltered)

    #message(headers_unfiltered=${headers_unfiltered})

    set(headers_filtered "")

    glare_apply_ignore_list("${headers_unfiltered}" "${ignored_paths}" headers_filtered)

    #message(headers_filtered=${headers_filtered})

    set(${component_headers_out} "${headers_filtered}" PARENT_SCOPE)
endfunction()

function(glare_gather_event_headers event_headers_out base_path ignored_paths)
    #message(base_path=${base_path})

    set(based_on_file_name "")

    glare_gather_headers_basic("${base_path}" "on_*" based_on_file_name)

    #message(based_on_file_name=${based_on_file_name})

    set(based_on_directory "")

    # Disabled for now; significant slow down due to scanning `build` directory.
    # Since events tend to be in `on_*.hpp` format, this doesn't really pick up anything missing from `based_on_file_name` either.
    # TODO: Review again if we add events with different naming scheme.
    ##glare_gather_headers_advanced("${base_path}" "events/" based_on_directory)

    #message(based_on_directory=${based_on_directory})

    set(headers_unfiltered
        ${based_on_file_name}
        ${based_on_directory}
    )

    ##list(REMOVE_DUPLICATES headers_unfiltered)

    #message(headers_unfiltered=${headers_unfiltered})

    set(headers_filtered "")

    glare_apply_ignore_list("${headers_unfiltered}" "${ignored_paths}" headers_filtered)

    #message(headers_filtered=${headers_filtered})

    set(${event_headers_out} "${headers_filtered}" PARENT_SCOPE)
endfunction()

function(glare_gather_reflection_headers reflection_headers_out base_path ignored_paths)
    set(headers_unfiltered "")

    glare_gather_headers_basic("${base_path}" "reflection" headers_unfiltered)

    #message(headers_unfiltered=${headers_unfiltered})

    set(headers_filtered "")

    glare_apply_ignore_list("${headers_unfiltered}" "${ignored_paths}" headers_filtered)

    #message(headers_filtered=${headers_filtered})

    set(${reflection_headers_out} "${headers_filtered}" PARENT_SCOPE)
endfunction()