add_executable(glare) # glare_exe

target_sources(
    glare # glare_exe

    PUBLIC

    "game.cpp"
)

target_link_libraries(
    glare # glare_exe
    
    PUBLIC
    
    glare_engine
    glare_core
    glare_graphics
    
    #glare_common
    #glare_util
)

#target_link_libraries(
#    glare
#    
#    PUBLIC
#    
#    glare_util
#    glare_math
#    glare_app
#    glare_graphics
#    glare_engine
#    glare_editor
#    glare_game
#
#    #PRIVATE
#    glare_third_party
#
#    #glare_common
#)

#target_compile_features(glare PRIVATE cxx_std_23) # INTERFACE

add_executable(glare::exe ALIAS glare) # glare_exe

set_property(TARGET glare PROPERTY OUTPUT_NAME glare) # glare_exe

if(MSVC)
    #if($<CONFIG:Debug>)
        target_link_options(glare PUBLIC /DEBUG:FASTLINK) # glare_exe
    #endif()
endif()

target_compile_features(glare PRIVATE cxx_std_23) # glare_exe

# Post-build copy

#if(MSVC)
    add_custom_command(
        TARGET glare

        POST_BUILD

        COMMAND ${CMAKE_COMMAND} -E copy_directory
                ${PROJECT_SOURCE_DIR}/glare/glare/
                $<TARGET_FILE_DIR:glare>
    )
#endif()

#target_link_libraries(
#    glare # glare_exe
#    
#    PUBLIC
#    
#    glare::glare
#    
#    #glare_common
#    #glare_util
#)