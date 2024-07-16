add_library(glare_app OBJECT) # STATIC # SHARED

target_sources(
    glare_app
    
    PRIVATE

    "application.cpp"
    
    "graphics_application.cpp"
    "window.cpp"
)

add_subdirectory("input")

if((GLARE_USE_UNITY_BUILD) AND (NOT GLARE_APP_USE_UNITY_BUILD))
    set(GLARE_APP_USE_UNITY_BUILD ON)
    set(GLARE_APP_UNITY_BUILD_GRANULAR ON)
endif()

if(GLARE_APP_USE_UNITY_BUILD)
    glare_configure_unity_build_for_module(glare_app "${CMAKE_CURRENT_SOURCE_DIR}" false "${GLARE_APP_UNITY_BUILD_GRANULAR}")
endif()

target_link_libraries(
    glare_app

    PUBLIC
    
    glare_core
    glare_third_party
    glare_graphics
)

find_package(EnTT CONFIG REQUIRED)

target_link_libraries(glare_app PRIVATE EnTT::EnTT)

find_package(SDL2 CONFIG REQUIRED)

target_link_libraries(
    glare_app
    
    PRIVATE

    $<TARGET_NAME_IF_EXISTS:SDL2::SDL2main>
    $<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>
)

find_package(imgui CONFIG REQUIRED)

target_link_libraries(glare_app PRIVATE imgui::imgui)