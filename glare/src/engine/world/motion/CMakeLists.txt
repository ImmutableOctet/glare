#add_library(glare_engine_world_motion OBJECT) # STATIC

target_sources(
    glare_engine # glare_engine_world_motion
    
    PRIVATE

    # Components:
    # Nothing so far.

    # Top-Level:
    "motion_system.cpp"
    "reflection.cpp"
)

#target_link_libraries(
#    glare_engine_world_motion
#
#    PRIVATE
#
#    glare_engine_world_common
#)