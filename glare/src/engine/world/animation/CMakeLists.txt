#add_library(glare_engine_world_animation OBJECT) # STATIC

target_sources(
    glare_engine # glare_engine_world_animation
    
    PRIVATE

    # Components:
    "components/animation_component.cpp"
    "components/bone_component.cpp"
    "components/skeletal_component.cpp"

    # Top-Level:
    "animation_system.cpp"
    "reflection.cpp"
)

#target_link_libraries(
#    glare_engine_world_animation
#
#    PRIVATE
#
#    glare_engine_world_common
#)