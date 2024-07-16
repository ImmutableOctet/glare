#add_library(glare_engine_world_camera OBJECT) # STATIC

target_sources(
    glare_engine # glare_engine_world_camera
    
    PRIVATE

    "camera_system.cpp"
    #"reflection.cpp"
)

#target_link_libraries(
#    glare_engine_world_camera
#
#    PRIVATE
#
#    glare_engine_world_common
#)