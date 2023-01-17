# NOTE: May change to `DYNAMIC` later.
#add_library(glare_engine_debug OBJECT) # STATIC

target_sources(
    glare_engine # glare_engine_debug
    
    PRIVATE

    "debug.cpp"
    "reflection.cpp"
)

#target_link_libraries(
#    glare_engine_debug
#
#    PRIVATE
#
#    glare_engine_common
#    #glare_engine_world
#)