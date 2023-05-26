#add_library(glare_app OBJECT) # STATIC

target_sources(
    glare_core # glare_app
    
    PRIVATE

    "application.cpp"
    
    "graphics_application.cpp"
    "window.cpp"
)

add_subdirectory("input")

#add_dependencies(
#    glare_app
#    glare_common
#)
#
#target_link_libraries(
#    glare_app
#
#    PRIVATE
#    
#    glare_common
#    glare_graphics
#    glare_math
#    glare_util
#    glare_third_party
#)

find_package(EnTT CONFIG REQUIRED)
target_link_libraries(glare_core PUBLIC EnTT::EnTT) # glare_app

#find_package(SDL2 CONFIG REQUIRED)
#target_link_libraries(glare_core
#    PRIVATE
#    $<TARGET_NAME_IF_EXISTS:SDL2::SDL2main>
#    $<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>
#)

find_package(imgui CONFIG REQUIRED)
target_link_libraries(glare_core PUBLIC imgui::imgui) # glare_app