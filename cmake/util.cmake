function (glare_snake_case_to_camel_case snake_case_value camel_case_out)
    string(REGEX REPLACE "\_?([a-z])([a-z]*)" "\\1;" data_to_upper ${snake_case_value})
    string(REGEX REPLACE "\_?([a-z])([a-z]*)" "\\2;" data_lowercase ${snake_case_value})

    set(camel_case_value "")

    foreach(leading trailing_lower IN ZIP_LISTS data_to_upper data_lowercase)
        #message(leading=${leading})
        #message(trailing_lower=${trailing_lower})

        string(TOUPPER "${leading}" leading_upper)
            
        set(camel_case_value "${camel_case_value}${leading_upper}${trailing_lower}")
    endforeach()

    set(${camel_case_out} "${camel_case_value}" PARENT_SCOPE)
endfunction()

function(glare_camel_case_names_from_snake_case_file_list file_list names_out)
    set(pending_names "")
    
    foreach(file_path ${file_list})
        get_filename_component(file_name "${file_path}" NAME_WLE)

        set(camel_case_name "")

        glare_snake_case_to_camel_case(${file_name} camel_case_name)

        list(APPEND pending_names "${camel_case_name}")
    endforeach()

    list(REMOVE_DUPLICATES pending_names)

    set(${names_out} "${pending_names}" PARENT_SCOPE)
endfunction()