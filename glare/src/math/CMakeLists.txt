#add_library(glare_math OBJECT) # STATIC

target_sources(
    glare_core # glare_math
    
    PRIVATE
    
    "math.cpp"

    "aabb.cpp"
    "bullet.cpp"
    "joyhat.cpp"
    "conversion.cpp"
)

#target_include_directories(
#    glare_math ${warning_guard}
#    PUBLIC
#    "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>"
#)
#
#target_compile_features(glare_math PRIVATE cxx_std_23)
#
#add_dependencies(
#    glare_math
#    glare_util # glare_common
#)
#
#target_link_libraries(
#    glare_math
#    
#    PRIVATE
#
#    glare_util # glare_common
#)

find_package(glm CONFIG REQUIRED)
target_link_libraries(glare_core PUBLIC glm::glm) # glare_math