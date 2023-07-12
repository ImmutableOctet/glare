#add_library(glare_engine_history OBJECT) # STATIC

target_sources(
    glare_engine # glare_engine_history
    
    PRIVATE
    
    "history_system.cpp"
)