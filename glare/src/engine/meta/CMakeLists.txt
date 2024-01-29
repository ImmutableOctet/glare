#add_library(glare_engine_meta OBJECT) # STATIC

target_sources(
    glare_engine # glare_engine_meta
    
    PRIVATE

    "runtime_traits.cpp"
    "component.cpp"
    "serial.cpp"
    "listen.cpp"

    "meta_type_resolution_context.cpp"
    "component_storage.cpp"
    "indirect_meta_data_member.cpp"
    "meta_data_member.cpp"
    "meta_event_listener.cpp"
    "meta_type_descriptor.cpp"
    "meta_variable.cpp"
    "reflection.cpp"
    "meta_value_operation.cpp"
    "meta_function_call.cpp"
    "meta_type_reference.cpp"
    "indirect_meta_any.cpp"
    "hash.cpp"
    
    "meta_variable_context.cpp"
    "meta_variable_evaluation_context.cpp"
    "meta_variable_target.cpp"
    "meta_any_wrapper.cpp"
    "meta_type_conversion.cpp"
    "meta_evaluation_context.cpp"
    "meta_evaluation_context_store.cpp"
    "meta_value_operator.cpp"
    "indirect_meta_variable_target.cpp"
    "indirection.cpp"
    "string.cpp"
    "data_member.cpp"
    "function.cpp"
    "cast.cpp"
    "meta_property.cpp"
)

#target_link_libraries(
#    glare_engine_meta
#
#    PRIVATE
#
#    glare_engine_common
#)