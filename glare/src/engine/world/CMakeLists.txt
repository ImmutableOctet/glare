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
    "scene.cpp"
)

add_subdirectory("components")

add_subdirectory("common")

add_subdirectory("controls")

add_subdirectory("animation")
add_subdirectory("behaviors")
add_subdirectory("camera")
add_subdirectory("motion")
add_subdirectory("physics")
add_subdirectory("render")
add_subdirectory("delta")

#target_link_libraries(
#    glare_engine_world
#
#    PUBLIC # PRIVATE
#
#    glare_engine_world_common # PRIVATE
#
#    glare_engine_world_controls
#
#    glare_engine_world_animation
#    glare_engine_world_behaviors
#    glare_engine_world_motion
#    glare_engine_world_physics
#    glare_engine_world_render
#)