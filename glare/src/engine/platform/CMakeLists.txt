if (WIN32)
    target_sources(
        glare_engine # glare_engine_entity
    
        PRIVATE

        "win32/reflection.cpp"
    )
endif()