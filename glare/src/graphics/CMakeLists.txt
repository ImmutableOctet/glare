#add_library(glare_graphics OBJECT) # STATIC
#add_library(${PROJECT_NAME}::graphics ALIAS glare_graphics)

target_sources(
    glare_graphics
    
    PRIVATE # PUBLIC

    #"context.cpp"

    "context_state.cpp"
    "canvas.cpp"

    # TODO: Move Bullet-related functionality out of this module.
    "bullet_debug.cpp"
    "collision_geometry.cpp"

    "types.cpp"
    "vertex.cpp"
    "resource.cpp"
    "skeleton.cpp"
    "animation.cpp"
    "cubemap_transform.cpp"
    "framebuffer.cpp"
    "renderbuffer.cpp"
    "gbuffer.cpp"
    "material.cpp"
    "texture.cpp"
    "mesh.cpp"
    "model.cpp"
    "pixelmap.cpp"
    "shader.cpp"
    "shadow_map.cpp"
)

add_subdirectory("drivers")

target_include_directories(
    glare_graphics
    ${warning_guard}
    PUBLIC
    "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/glare/src>"
)

#add_dependencies(
#    glare_graphics
#    glare_math
#    glare_util
#    #glare_common
#    #glare_app
#)

target_link_libraries(
    glare_graphics

    PRIVATE
    glare_core
)

#target_link_libraries(
#    glare_graphics
#    
#    PRIVATE
#
#    glare_math
#    glare_util
#
#    # Embedded third-party libraries.
#    # (e.g. ImGui's backend files)
#    glare_third_party
#)

find_package(GLEW REQUIRED)
target_link_libraries(glare_graphics PUBLIC GLEW::GLEW) # PRIVATE

#find_package(SDL2 CONFIG REQUIRED)
#target_link_libraries(glare_graphics
#    PRIVATE # PUBLIC
#    $<TARGET_NAME_IF_EXISTS:SDL2::SDL2main>
#    $<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>
#)

#find_package(glm CONFIG REQUIRED)
#target_link_libraries(glare_graphics PUBLIC glm::glm) # PRIVATE

# Needed due to `bullet_debug` and `collision_geometry` source files.
# TODO: Move to a more appropriate location.
find_package(Bullet CONFIG REQUIRED)
target_link_libraries(glare_graphics PUBLIC ${BULLET_LIBRARIES}) # PRIVATE

# TODO: Look into separating ImGui from this module.
find_package(imgui CONFIG REQUIRED)
target_link_libraries(glare_graphics PUBLIC imgui::imgui)