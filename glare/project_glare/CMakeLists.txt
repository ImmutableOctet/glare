add_executable(glare)

set_target_properties(glare PROPERTIES ENABLE_EXPORTS 1)

target_sources(
    glare

    PUBLIC

    "game.cpp"
)

target_link_libraries(
    glare
    
    PUBLIC
    
    glare_app
    glare_engine
    glare_core
    glare_graphics
    glare_script
)

set_target_properties(glare PROPERTIES LINK_DEPENDS_NO_SHARED true)

#target_compile_features(glare PRIVATE cxx_std_23) # INTERFACE

add_executable(glare::exe ALIAS glare)

set_property(TARGET glare PROPERTY OUTPUT_NAME glare)

set_property(TARGET glare PROPERTY CXX_STANDARD 23)

#cmake_policy(SET CMP0067 NEW)
set_property(TARGET glare PROPERTY CXX_STANDARD_REQUIRED ON)

target_compile_options(glare PRIVATE "/std:c++latest")

#if(MSVC)
#    #if($<CONFIG:Debug>)
#        target_link_options(glare PUBLIC /DEBUG:FASTLINK /INCREMENTAL)
#        target_compile_options(glare PUBLIC /ZI)
#    #endif()
#endif()

target_compile_features(glare PRIVATE cxx_std_23)

if (GLARE_SCRIPT_USE_CLING)
    add_custom_command(
        TARGET glare POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_RUNTIME_DLLS:glare> $<TARGET_FILE_DIR:glare>
        COMMAND_EXPAND_LISTS
    )
endif()

set(GLARE_ENGINE_HEADER_PATH "${CMAKE_SOURCE_DIR}/glare/src/engine")

#set(GLARE_POST_BUILD_COPY_FILES ON)

# If this is enabled, and `GLARE_POST_BUILD_COPY_FILES` is disabled, symlink creation will be attempted.
set(GLARE_POST_BUILD_ALLOW_SYMLINKS ON)

#set(GLARE_POST_BUILD_COPY_HEADERS ON)

# Post-build copy from `glare` directory to destination:
if (GLARE_POST_BUILD_COPY_FILES)
    add_custom_command(
        TARGET glare

        POST_BUILD

        # TODO: Change to `copy_directory_if_different` after upgrading CMake to 3.26.
        COMMAND ${CMAKE_COMMAND} -E copy_directory
                ${PROJECT_SOURCE_DIR}/glare/glare/
                $<TARGET_FILE_DIR:glare>

        COMMENT "Copying files..."
    )
elseif(GLARE_POST_BUILD_ALLOW_SYMLINKS)
    # NOTE: Manual symlink creation. This is a temporary solution to avoid file enumeration hacks.
    # We only have the `GLARE_POST_BUILD_COPY_FILES` variable to speed up builds/iteration, but when we switch to CMake 3.26, symlinks may not be needed.
    add_custom_command(
        TARGET glare

        POST_BUILD

        COMMAND ${CMAKE_COMMAND} -E create_symlink
                ${PROJECT_SOURCE_DIR}/glare/glare/assets
                $<TARGET_FILE_DIR:glare>/assets

        COMMAND ${CMAKE_COMMAND} -E create_symlink
                ${PROJECT_SOURCE_DIR}/glare/glare/config
                $<TARGET_FILE_DIR:glare>/config

        COMMAND ${CMAKE_COMMAND} -E create_symlink
                ${PROJECT_SOURCE_DIR}/glare/glare/engine
                $<TARGET_FILE_DIR:glare>/engine

        COMMAND ${CMAKE_COMMAND} -E create_symlink
                ${PROJECT_SOURCE_DIR}/glare/glare/objects
                $<TARGET_FILE_DIR:glare>/objects

        COMMENT "Creating symlinks..."
    )
endif()

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
            TARGET glare
            POST_BUILD
            
            COMMAND ${CMAKE_COMMAND} -E copy
                    ${GLARE_ENGINE_HEADER_PATH}/${header_file}
                    "$<TARGET_FILE_DIR:glare>/engine/script/include/engine/${header_file}"
        )
    endforeach()

    find_package(EnTT CONFIG REQUIRED)
    #target_link_libraries(glare PUBLIC EnTT::EnTT)

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
            TARGET glare
            POST_BUILD
            
            COMMAND ${CMAKE_COMMAND} -E copy
                    ${GLARE_ENTT_INCLUDE_PATH}/${header_file}
                    "$<TARGET_FILE_DIR:glare>/engine/script/include/entt/${header_file}"
        )
    endforeach()
endif()

#target_link_libraries(
#    glare
#    
#    PUBLIC
#    
#    glare::glare
#    
#    #glare_common
#    #glare_util
#)