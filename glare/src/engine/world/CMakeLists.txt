#add_library(glare_engine_world STATIC) # OBJECT

target_sources(
    glare_engine # glare_engine_world
    
    PRIVATE

    "world.cpp"
    "reflection.cpp"
    "world_system.cpp"
    "entity.cpp"
    "graphics_entity.cpp"
    "light.cpp"
    "stage.cpp"
)

add_subdirectory("components")

add_subdirectory("common")

# TODO: Rename `meta` subdirectory to something better. (`meta_controls`, etc.)
add_subdirectory("meta_controls")

add_subdirectory("animation")
add_subdirectory("behaviors")
add_subdirectory("motion")
add_subdirectory("physics")
add_subdirectory("render")

#target_link_libraries(
#    glare_engine_world
#
#    PUBLIC # PRIVATE
#
#    glare_engine_world_common # PRIVATE
#
#    glare_engine_world_meta_controls
#
#    glare_engine_world_animation
#    glare_engine_world_behaviors
#    glare_engine_world_motion
#    glare_engine_world_physics
#    glare_engine_world_render
#)