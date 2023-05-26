#add_library(glare_engine_world_delta OBJECT) # STATIC

target_sources(
    glare_engine # glare_engine_world_delta
    
    PRIVATE

    "delta_system.cpp"
    "reflection.cpp"
)

#target_link_libraries(
#    glare_engine_world_delta
#
#    PRIVATE
#
#    glare_engine_world_common
#)