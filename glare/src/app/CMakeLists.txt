add_library(glare_app OBJECT) # STATIC # SHARED

target_sources(
    glare_app
    
    PRIVATE

    "application.cpp"
    
    "graphics_application.cpp"
    "window.cpp"
)

add_subdirectory("input")

target_link_libraries(
    glare_app

    PUBLIC
    
    glare_core
    glare_third_party
    glare_graphics
)

find_package(EnTT CONFIG REQUIRED)

target_link_libraries(glare_app PUBLIC EnTT::EnTT)

find_package(SDL2 CONFIG REQUIRED)

target_link_libraries(
    glare_app
    
    PUBLIC

    $<TARGET_NAME_IF_EXISTS:SDL2::SDL2main>
    $<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>
)

find_package(imgui CONFIG REQUIRED)

target_link_libraries(glare_app PUBLIC imgui::imgui)