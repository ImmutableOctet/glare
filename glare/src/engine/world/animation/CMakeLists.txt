#add_library(glare_engine_world_animation OBJECT) # STATIC

target_sources(
    glare_engine # glare_engine_world_animation
    
    PRIVATE

    "components/animation_component.cpp"
    "components/bone_component.cpp"
    "components/skeletal_component.cpp"

    "animation_system.cpp"
    "animation_layer.cpp"
    "animation_state.cpp"
    "animation_repository.cpp"
    "skeletal_key_sequence.cpp"
    "skeleton.cpp"

    "reflection.cpp"
)

#target_link_libraries(
#    glare_engine_world_animation
#
#    PRIVATE
#
#    glare_engine_world_common
#)