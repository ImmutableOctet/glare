if(WIN32)
    target_sources(
        glare_app

        PRIVATE

        "win32/platform_extensions.cpp"
    )

    target_link_libraries(glare_app PRIVATE "winmm.lib")
else()
    target_sources(
        glare_app

        PRIVATE

        "stub/platform_extensions.cpp"
    )
endif()