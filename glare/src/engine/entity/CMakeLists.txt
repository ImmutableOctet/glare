#add_library(glare_engine_entity OBJECT) # STATIC

target_sources(
    glare_engine # glare_engine_entity
    
    PRIVATE

    # Commands:
    # Nothing so far.

    # Components:
    "components/entity_thread_component.cpp"

    # Top-level:
    "entity_descriptor.cpp"
    "entity_factory.cpp"
    "entity_listener.cpp"
    "entity_state.cpp"
    "entity_system.cpp"
    "entity_target.cpp"
    "entity_thread.cpp"
    "entity_thread_builder.cpp"
    "event_trigger_condition.cpp"
    "state_storage_manager.cpp"

    "reflection.cpp"
    "entity_factory_context.cpp"
    "serial.cpp"
    "entity_state_action.cpp"
    "entity_thread_target.cpp"
    "meta_description.cpp"
  "components/instance_component.cpp" "components/entity_context_component.cpp" "entity_context.cpp"  "parse.cpp")

#target_link_libraries(
#    glare_engine_entity
#
#    PRIVATE
#
#    glare_engine_common
#    glare_engine_meta
#)