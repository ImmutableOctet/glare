#add_library(glare_util OBJECT) # STATIC

target_sources(
    glare_core # glare_util
    
    PRIVATE
    
    "io.cpp"
    "json.cpp"
    "string.cpp"
    "parse.cpp"
)