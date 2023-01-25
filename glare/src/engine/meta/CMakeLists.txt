#add_library(glare_engine_meta OBJECT) # STATIC

target_sources(
    glare_engine # glare_engine_meta
    
    PRIVATE

    "meta.cpp"
    "serial.cpp"

    "parsing_context.cpp"
    "component_storage.cpp"
    "indirect_meta_data_member.cpp"
    "meta_data_member.cpp"
    "meta_description.cpp"
    "meta_event_listener.cpp"
    "meta_type_descriptor.cpp"
    "meta_variable.cpp"

    "reflection.cpp"
)

#target_link_libraries(
#    glare_engine_meta
#
#    PRIVATE
#
#    glare_engine_common
#)