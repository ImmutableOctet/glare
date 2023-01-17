#add_library(glare_game OBJECT) # STATIC

target_sources(
    glare_engine # glare_core # glare # glare_game
    
    PRIVATE

    "effects.cpp"
    "game.cpp"
    "screen.cpp"
)

#target_link_libraries(
#    glare_game
#    
#    PRIVATE
#
#    glare_common
#    glare_app
#    glare_engine
#    glare_graphics
#    glare_math
#    glare_util
#)