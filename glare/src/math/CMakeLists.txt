#add_library(glare_math OBJECT) # STATIC

target_sources(
    glare_core # glare_math
    
    PRIVATE
    
    "aabb.cpp"
    "bullet.cpp"
    "conversion.cpp"
    "joyhat.cpp"
    "lerp.cpp"
    "rotation.cpp"
)