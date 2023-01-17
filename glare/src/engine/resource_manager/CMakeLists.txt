#add_library(glare_engine_resource_manager OBJECT) # STATIC

target_sources(
    glare_engine # glare_engine_resource_manager
    
    PRIVATE

    # Loaders:
    "loaders/model_loader.cpp"

    # Top-level:
    "animation_data.cpp"
    "collision_data.cpp"
    "entity_factory_data.cpp"
    "resource_manager.cpp"
)

#target_link_libraries(
#    glare_engine_resource_manager
#
#    PRIVATE
#
#    glare_engine_common
#    ##glare_engine_entity
#)

find_package(assimp CONFIG REQUIRED)
target_link_libraries(glare_engine PUBLIC assimp::assimp) # glare_engine_resource_manager