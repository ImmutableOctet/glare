# Editor
add_executable(glare_scene_editor)

target_sources(
    glare_scene_editor

    PUBLIC

    "scene_editor.cpp"
)

target_link_libraries(
    glare_scene_editor
    
    PUBLIC
    
    glare_engine
    glare_core
    glare_graphics
    
    #glare_common
    #glare_util
)

set_property(TARGET glare_scene_editor PROPERTY OUTPUT_NAME glare_scene_editor)

if(MSVC)
    #if($<CONFIG:Debug>)
        target_link_options(glare_scene_editor PUBLIC /DEBUG:FASTLINK)
    #endif()
endif()

target_compile_features(glare_scene_editor PRIVATE cxx_std_23)

# Post-build copy

#if(MSVC)
    add_custom_command(
        TARGET glare_scene_editor

        POST_BUILD

        COMMAND ${CMAKE_COMMAND} -E copy_directory
                ${PROJECT_SOURCE_DIR}/glare/glare/
                $<TARGET_FILE_DIR:glare_scene_editor>
    )
#endif()