#add_library(glare_engine_world_meta_controls OBJECT) # STATIC

target_sources(
    glare_engine # glare_engine_world_meta_controls
    
    PRIVATE

    "meta_controls.cpp"
)

#target_link_libraries(
#    glare_engine_world_meta_controls
#
#    PRIVATE
#
#    glare_engine_world_common
#)