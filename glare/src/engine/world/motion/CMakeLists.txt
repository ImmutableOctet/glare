#add_library(glare_engine_world_motion OBJECT) # STATIC

target_sources(
    glare_engine # glare_engine_world_motion
    
    PRIVATE

    "motion_system.cpp"
    "components/orientation_component.cpp"
    "components/rotate_component.cpp"
    "reflection.cpp"
)

#target_link_libraries(
#    glare_engine_world_motion
#
#    PRIVATE
#
#    glare_engine_world_common
#)