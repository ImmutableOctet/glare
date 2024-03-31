add_library(glare_engine OBJECT) # STATIC # SHARED
add_library(${PROJECT_NAME}::engine ALIAS glare_engine)

set_target_properties(glare_engine PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS 1)

target_compile_features(glare_engine PRIVATE cxx_std_23)

target_sources(
    glare_engine
    
    PRIVATE
    
    "service.cpp"
    "timer.cpp"
    "transform.cpp"
)

#add_library(glare_engine_reflection STATIC)
#add_library(${PROJECT_NAME}::engine_reflection ALIAS glare_engine_reflection)

#set_target_properties(
#    glare_engine # glare_engine_reflection
#    
#    PROPERTIES
#
#    UNITY_BUILD ON
#)
#
#set_target_properties(
#    glare_engine # glare_engine_reflection
#    
#    PROPERTIES
#
#    UNITY_BUILD_MODE GROUP
#)
#
#file(
#    GLOB_RECURSE glare_engine_reflection_sources
#    ${CMAKE_SOURCE_DIR}/glare/src/engine/*/reflection.cpp
#)
#
#message("`engine` reflection source files:")
#
#foreach(reflection_file ${glare_engine_reflection_sources})
#    set_source_files_properties(
#        ${reflection_file}
#        
#        PROPERTIES
#
#        UNITY_GROUP "reflection_unity_group"
#    )
#    
#    set_source_files_properties(
#        ${reflection_file}
#    
#        PROPERTIES
#    
#        # NOTE: This is a workaround to disable compilation as separate files.
#        # (i.e. this ignores `reflection` files in `target_sources` lists, if present)
#        #HEADER_FILE_ONLY TRUE
#    )
#
#    message(reflection_file=${reflection_file})
#
#    #target_sources(
#    #    glare_engine # glare_engine_reflection
#    #    PRIVATE
#    #    ${reflection_file}
#    #)
#endforeach()

#set_target_properties(glare_engine PROPERTIES ENABLE_EXPORTS 1)
##set_target_properties(glare_engine PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS 1)

find_package(SDL2 CONFIG REQUIRED)

target_link_libraries(glare_engine
    PRIVATE # PUBLIC
    $<TARGET_NAME_IF_EXISTS:SDL2::SDL2main>
    $<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>
)

target_link_libraries(
    glare_engine

    PRIVATE
    
    glare_core
    glare_graphics
    glare_app

    #glare_engine_reflection
)

#add_subdirectory("common")
add_subdirectory("meta")
add_subdirectory("components")
add_subdirectory("debug")
add_subdirectory("entity") 
add_subdirectory("input")
add_subdirectory("history")
add_subdirectory("resource_manager")
add_subdirectory("world")
add_subdirectory("editor")
add_subdirectory("script")
add_subdirectory("platform")
add_subdirectory("test")
add_subdirectory("reflection")