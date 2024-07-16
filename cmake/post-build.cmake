set(GLARE_ENGINE_HEADER_PATH "${CMAKE_SOURCE_DIR}/glare/src/engine")

#set(GLARE_POST_BUILD_COPY_FILES ON)

# If this is enabled, and `GLARE_POST_BUILD_COPY_FILES` is disabled, symlink creation will be attempted.
set(GLARE_POST_BUILD_ALLOW_SYMLINKS ON)

#set(GLARE_POST_BUILD_COPY_HEADERS ON)

function(glare_post_build_copy_runtime_libraries build_target)
    if (GLARE_SCRIPT_USE_CLING)
        add_custom_command(
            TARGET ${build_target} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_RUNTIME_DLLS:${build_target}> $<TARGET_FILE_DIR:${build_target}>
            COMMAND_EXPAND_LISTS
        )
    endif()
endfunction()

function(glare_post_build_copy_directory_ex source_target build_target source_directory_path destination_directory_path)
    # Post-build copy from `glare` directory to destination:
    if (GLARE_POST_BUILD_COPY_FILES)
        #add_custom_command(
        #    TARGET ${build_target}
        #
        #    POST_BUILD
        #
        #    # TODO: Change to `copy_directory_if_different` after upgrading CMake to 3.26.
        #    COMMAND ${CMAKE_COMMAND} -E copy_directory
        #            ${PROJECT_SOURCE_DIR}/shared/
        #            $<TARGET_FILE_DIR:${build_target}>
        #
        #    COMMENT "Copying files..."
        #)
    elseif(GLARE_POST_BUILD_ALLOW_SYMLINKS)
        get_target_property(target_source_directory ${source_target} SOURCE_DIR)
        
        #message(target_source_directory="${target_source_directory}")
        #message("Symlink source:")
        #message(${target_source_directory}/${source_directory_path})

        # NOTE: Manual symlink creation. This is a temporary solution to avoid file enumeration hacks.
        # We only have the `GLARE_POST_BUILD_COPY_FILES` variable to speed up builds/iteration, but when we switch to CMake 3.26, symlinks may not be needed.
        add_custom_command(
            TARGET ${build_target}
            
            POST_BUILD

            COMMAND ${CMAKE_COMMAND} -E create_symlink
                    ${target_source_directory}/${source_directory_path}
                    $<TARGET_FILE_DIR:${build_target}>/${destination_directory_path}
            
            COMMENT "Creating symlink for \"${source_directory_path}\"..."
            #COMMENT "Creating symlink for: ${destination_directory_path} -> ${PROJECT_SOURCE_DIR}/${source_directory_path}"
        )
    endif()
endfunction()

function(glare_post_build_copy_directory build_target directory_path)
    glare_post_build_copy_directory_ex(${build_target} ${build_target} ${directory_path} ${directory_path})
endfunction()

function(glare_post_build_copy_shared_files build_target)
    #glare_post_build_copy_directory_ex(glare_core ${build_target} "shared/engine" "engine")
    glare_post_build_copy_directory_ex(glare_core ${build_target} "../../engine" "engine")
    glare_post_build_copy_directory_ex(glare_core ${build_target} "../../licenses" "licenses")
endfunction()

function(glare_post_build_copy_headers build_target header_output_path) # engine/script/include
    # Copy header files from `engine` into `script/include` directory:
    if (GLARE_SCRIPT_USE_CLING AND GLARE_POST_BUILD_COPY_HEADERS)
        file(
            GLOB_RECURSE GLARE_ENGINE_HEADERS
            RELATIVE ${GLARE_ENGINE_HEADER_PATH}
    
            ${GLARE_ENGINE_HEADER_PATH}/*.hpp
            ${GLARE_ENGINE_HEADER_PATH}/*.h
        )
    
        #message("`engine` header files:")
    
        foreach(header_file ${GLARE_ENGINE_HEADERS})
            #message("${header_file} -> engine/script/include/engine/${header_file}")
    
            add_custom_command(
                TARGET ${build_target}
                POST_BUILD
            
                COMMAND ${CMAKE_COMMAND} -E copy
                        ${GLARE_ENGINE_HEADER_PATH}/${header_file}
                        "$<TARGET_FILE_DIR:${build_target}>/${header_output_path}/engine/${header_file}"
            )
        endforeach()

        find_package(EnTT CONFIG REQUIRED)
        #target_link_libraries(${build_target} PRIVATE EnTT::EnTT)

        #set(GLARE_ENTT_INCLUDE_PATH ${EnTT_SOURCE_DIR})
        get_target_property(GLARE_ENTT_PARENT_INCLUDE_PATH EnTT::EnTT INTERFACE_INCLUDE_DIRECTORIES)

        set(GLARE_ENTT_INCLUDE_PATH ${GLARE_ENTT_PARENT_INCLUDE_PATH}/entt)

        message(GLARE_ENTT_INCLUDE_PATH="${GLARE_ENTT_INCLUDE_PATH}")

        file(
            GLOB_RECURSE GLARE_ENTT_HEADERS
            RELATIVE ${GLARE_ENTT_INCLUDE_PATH}
    
            ${GLARE_ENTT_INCLUDE_PATH}/*.hpp
            ${GLARE_ENTT_INCLUDE_PATH}/*.h
        )

        message(GLARE_ENTT_HEADERS="${GLARE_ENTT_HEADERS}")

        #message("`entt` header files:")

        foreach(header_file ${GLARE_ENTT_HEADERS})
            #message("${header_file} -> engine/script/include/entt/${header_file}")
    
            add_custom_command(
                TARGET ${build_target}
                POST_BUILD
            
                COMMAND ${CMAKE_COMMAND} -E copy
                        ${GLARE_ENTT_INCLUDE_PATH}/${header_file}
                        "$<TARGET_FILE_DIR:${build_target}>/${header_output_path}/entt/${header_file}"
            )
        endforeach()
    endif()
endfunction()