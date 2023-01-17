#add_library(glare_engine_world_render OBJECT) # STATIC

target_sources(
    glare_engine # glare_engine_world_render
    
    PRIVATE

    "render_phase.cpp"
    "world_renderer.cpp"

    "geometry_render_phase.cpp"
    "directional_shadows.cpp"
    "point_light_shadows.cpp"
    "deferred_shading.cpp"
    "bullet_debug_render_phase.cpp"
)

#target_link_libraries(
#    glare_engine_world_render
#
#    PRIVATE
#
#    glare_engine_world_common
#    #glare_engine_world_physics
#)

#find_package(Bullet CONFIG REQUIRED)
#target_link_libraries(glare_engine_world_render PUBLIC ${BULLET_LIBRARIES})