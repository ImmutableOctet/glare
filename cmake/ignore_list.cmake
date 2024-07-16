function(glare_ignore_list file_path file_contents_out)
	set(file_contents "")
	
	#message("Loading ignore list: \"${file_path}\"...")

	file(STRINGS "${file_path}" file_contents)
	set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${file_path}")

	set(${file_contents_out} "${file_contents}" PARENT_SCOPE)
endfunction()

function(glare_apply_ignore_list files_unfiltered ignored_paths files_out)
    set(files_filtered "")

    foreach(file_path ${files_unfiltered})
        set(file_ignored false)

        foreach (ignored_path ${ignored_paths})
            if("${file_path}" MATCHES "^${ignored_path}")
                set(file_ignored true)

                #message("Ignoring: \"${file_path}\" due to ignored path entry: \"${ignored_path}\"")

                break()
            endif()
        endforeach()

        if(NOT file_ignored)
            list(APPEND files_filtered "${file_path}")
        endif()
    endforeach()

    #message(files_filtered=${files_filtered})

    set(${files_out} "${files_filtered}" PARENT_SCOPE)
endfunction()