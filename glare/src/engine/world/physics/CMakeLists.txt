# add_library(glare_engine_world_physics OBJECT) # STATIC

target_sources(
    glare_engine # glare_engine_world_physics
    
    PRIVATE # PUBLIC

    # Components:
    "components/collision_component.cpp"

    # Top-Level:
    "physics_system.cpp"
    "ground.cpp"
    "collision_config.cpp"
    "collision_events.cpp"
    "collision_group.cpp"
    "collision_cast.cpp"
    "collision_motion_state.cpp"
    "kinematic_resolution_config.cpp"
    "reflection.cpp"
)

#set_property(TARGET glare_engine_world_physics PROPERTY CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

#target_link_libraries(
#    glare_engine_world_physics
#
#    PRIVATE
#
#    glare_engine_world_common
#)

find_package(Bullet CONFIG REQUIRED)
target_link_libraries(glare_engine PUBLIC ${BULLET_LIBRARIES}) # glare_engine_world_physics