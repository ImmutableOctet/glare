#add_library(glare_engine STATIC) # OBJECT
#add_library(${PROJECT_NAME}::engine ALIAS glare_engine)

target_sources(
    glare_engine
    
    PRIVATE

    "reflection.cpp"
    "config.cpp"
    "service.cpp"
    "timer.cpp"
    "transform.cpp"
)

target_link_libraries(
    glare_engine

    PRIVATE
    glare_core
    glare_graphics
)

#find_package(SDL2 CONFIG REQUIRED)
#target_link_libraries(glare_engine
#    PRIVATE # PUBLIC
#    $<TARGET_NAME_IF_EXISTS:SDL2::SDL2main>
#    $<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>
#)

add_subdirectory("components")

#add_subdirectory("common")

add_subdirectory("debug")
add_subdirectory("entity")
add_subdirectory("input")
add_subdirectory("meta")
add_subdirectory("resource_manager")
add_subdirectory("world")

#add_dependencies(
#    glare_engine
#    glare_engine_common
#)

#target_link_libraries(
#    glare_engine
#    
#    PUBLIC
#
#    glare_engine_meta
#    glare_engine_debug
#    glare_engine_entity
#    glare_engine_input
#    glare_engine_resource_manager
#    
#    glare_engine_world
#
#    # PRIVATE
#    glare_engine_common
#)