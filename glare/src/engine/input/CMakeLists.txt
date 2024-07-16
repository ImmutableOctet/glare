#add_library(glare_engine_input OBJECT) # STATIC

target_sources(
    glare_engine # glare_engine_input
    
    PRIVATE
    
    "input_event.cpp"
    "input_state.cpp"
    "input_system.cpp"

    "analog.cpp"

    # TODO: Move to another target, etc.
    #"reflection.cpp"
)

#target_link_libraries(
#    glare_engine_input
#
#    PRIVATE
#
#    glare_engine_common
#)