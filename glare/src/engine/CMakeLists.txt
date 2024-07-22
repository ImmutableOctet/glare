add_library(glare_engine OBJECT) # STATIC # SHARED

add_library(${PROJECT_NAME}::engine ALIAS glare_engine)

set_target_properties(glare_engine PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS 1)

if(GLARE_ENGINE_REFLECT_ALL_AUTOMATICALLY)
    set_target_properties(glare_engine PROPERTIES GLARE_ENGINE_REFLECT_ALL_AUTOMATICALLY 1)    
endif()

target_compile_features(glare_engine PRIVATE cxx_std_23)
set_property(TARGET glare_engine PROPERTY CXX_STANDARD 23)
set_property(TARGET glare_engine PROPERTY CXX_STANDARD_REQUIRED ON)

target_sources(
    glare_engine
    
    PRIVATE
    
    "service.cpp"
    "timer.cpp"
    "transform.cpp"
)

if((GLARE_USE_UNITY_BUILD) AND (NOT GLARE_ENGINE_USE_UNITY_BUILD))
    set(GLARE_ENGINE_USE_UNITY_BUILD ON)
    set(GLARE_ENGINE_UNITY_BUILD_GRANULAR ON)
endif()

if(GLARE_ENGINE_USE_UNITY_BUILD)
    glare_configure_unity_build_for_module(glare_engine "${CMAKE_CURRENT_SOURCE_DIR}" false "${GLARE_ENGINE_UNITY_BUILD_GRANULAR}")
endif()

#set(GLARE_REFLECTION_USE_UNITY_BUILD ON)

add_library(glare_engine_reflection OBJECT) # STATIC
add_library(${PROJECT_NAME}::engine_reflection ALIAS glare_engine_reflection)

target_compile_features(glare_engine_reflection PRIVATE cxx_std_23)
set_property(TARGET glare_engine_reflection PROPERTY CXX_STANDARD 23)
set_property(TARGET glare_engine_reflection PROPERTY CXX_STANDARD_REQUIRED ON)

target_link_libraries(
    glare_engine_reflection
    
    PUBLIC
    
    glare_core
    glare_engine
    glare_graphics
    glare_app
)

glare_add_reflection_sources_to_target(glare_engine_reflection "${PROJECT_SOURCE_DIR}/glare/src/engine" "${PROJECT_SOURCE_DIR}/glare/src/engine/reflection" "glare_reflection")

#set_target_properties(glare_engine PROPERTIES ENABLE_EXPORTS 1)
##set_target_properties(glare_engine PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS 1)

find_package(SDL2 CONFIG REQUIRED)

target_link_libraries(glare_engine
    PRIVATE # PUBLIC
    $<TARGET_NAME_IF_EXISTS:SDL2::SDL2main>
    $<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>
)

# Test
find_package(folly CONFIG REQUIRED)
target_link_libraries(glare_engine PRIVATE Folly::folly)



target_link_libraries(
    glare_engine

    PRIVATE
    
    glare_core
    glare_graphics
    glare_app

    #glare_engine_reflection
)

#add_subdirectory("common")
add_subdirectory("meta")
add_subdirectory("components")
add_subdirectory("debug")
add_subdirectory("entity") 
add_subdirectory("input")
add_subdirectory("history")
add_subdirectory("resource_manager")
add_subdirectory("world")
add_subdirectory("editor")
add_subdirectory("script")
add_subdirectory("platform")
add_subdirectory("test")
add_subdirectory("reflection")
