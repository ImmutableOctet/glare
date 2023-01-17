#add_library(glare_engine_world_behaviors OBJECT) # STATIC

target_sources(
    glare_engine # glare_engine_world_behaviors
    
    PRIVATE

    "bridge.cpp"
    "reflection.cpp"

    "billboard_behavior.cpp"
    "debug_move_behavior.cpp"
    "free_look_behavior.cpp"
    "rave_behavior.cpp"
    "simple_follow_behavior.cpp"
    "spin_behavior.cpp"
    "target_behavior.cpp"
)

#target_link_libraries(
#    glare_engine_world_behaviors
#
#    PRIVATE
#
#    glare_engine_world_common
#)