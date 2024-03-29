# Shared graphics library:
add_library(glare_graphics OBJECT) # STATIC # SHARED
add_library(${PROJECT_NAME}::graphics ALIAS glare_graphics)

set_target_properties(glare_graphics PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS 1)

target_compile_features(glare_graphics PRIVATE cxx_std_23)

target_sources(
    glare_graphics
    
    PRIVATE # PUBLIC

    #"context.cpp"

    "context_state.cpp"
    "canvas.cpp"

    # TODO: Move Bullet-related functionality out of this module.
    "bullet_debug.cpp"
    "collision_geometry.cpp"

    "point_rect.cpp"
    "vertex.cpp"
    "resource.cpp"
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
    "shader_source.cpp"
)

add_subdirectory("drivers")

target_include_directories(
    glare_graphics
    ${warning_guard}
    PUBLIC
    "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/glare/src>"
)

target_link_libraries(
    glare_graphics

    PRIVATE # PUBLIC

    glare_core
    glare_third_party
)

find_package(GLEW REQUIRED)
target_link_libraries(glare_graphics PRIVATE GLEW::GLEW)

find_package(SDL2 CONFIG REQUIRED)

target_link_libraries(glare_graphics
    PRIVATE
    $<TARGET_NAME_IF_EXISTS:SDL2::SDL2main>
    $<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>
)

#find_package(glm CONFIG REQUIRED)
#target_link_libraries(glare_graphics PRIVATE glm::glm)

# Needed due to `bullet_debug` and `collision_geometry` source files.
# TODO: Move to a more appropriate location.
find_package(Bullet CONFIG REQUIRED)
target_link_libraries(glare_graphics PRIVATE ${BULLET_LIBRARIES})

# TODO: Look into separating ImGui from this module.
find_package(imgui CONFIG REQUIRED)
target_link_libraries(glare_graphics PRIVATE imgui::imgui)