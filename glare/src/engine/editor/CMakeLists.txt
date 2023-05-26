#add_library(glare_engine_editor OBJECT) # STATIC

target_sources(
    glare_engine # glare_engine_editor
    
    PRIVATE

    "editor.cpp"
)

#target_link_libraries(
#    glare_engine_editor
#
#    PRIVATE
#
#    glare_engine_common
#    glare_engine_meta
#)