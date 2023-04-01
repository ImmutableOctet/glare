cmake_minimum_required(VERSION 3.14)

include(cmake/prelude.cmake)

include(cmake/project-is-top-level.cmake)
include(cmake/variables.cmake)

#set(CMAKE_BUILD_PARALLEL_LEVEL, 4)

#set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS, "ON")
#set(GLARE_BUILD_SHARED, "OFF")

if(GLARE_USE_CLANGCL)
    set(CMAKE_GENERATOR_TOOLSET "ClangCL")
endif()

project(
    glare
    VERSION 0.1.0
    DESCRIPTION "WIP Game Engine"
    HOMEPAGE_URL "https://github.com/ImmutableOctet/glare"
    LANGUAGES CXX
)

# ---- Declare executable ----

add_executable(glare) # glare_exe

target_sources(
    glare # glare_exe

    PUBLIC

    "glare/game.cpp"
)

target_link_libraries(
    glare # glare_exe
    
    PUBLIC
    
    glare_engine
    
    #glare_common
    #glare_util
)

add_executable(glare::exe ALIAS glare) # glare_exe

set_property(TARGET glare PROPERTY OUTPUT_NAME glare) # glare_exe

if(MSVC)
    #if($<CONFIG:Debug>)
        target_link_options(glare PUBLIC /DEBUG:FASTLINK) # glare_exe
    #endif()
endif()

target_compile_features(glare PRIVATE cxx_std_23) # glare_exe

add_subdirectory("glare/src")

#target_link_libraries(
#    glare # glare_exe
#    
#    PUBLIC
#    
#    glare::glare
#    
#    #glare_common
#    #glare_util
#)

#if(MSVC)
    add_custom_command(
        TARGET glare # glare_exe

        POST_BUILD

        #COMMAND ${CMAKE_COMMAND} -E xcopy ${PROJECT_SOURCE_DIR}/glare/assets ${CMAKE_BINARY_DIR}/assets /e /y /i /r /exclude:glare/exclude.txt

        COMMAND ${CMAKE_COMMAND} -E copy_directory
                ${CMAKE_SOURCE_DIR}/glare/glare/
                $<TARGET_FILE_DIR:glare> # glare_exe
    )
#endif()

# ---- Install rules ----

if(NOT CMAKE_SKIP_INSTALL_RULES)
  include(cmake/install-rules.cmake)
endif()

# ---- Developer mode ----

if(NOT glare_DEVELOPER_MODE)
  return()
elseif(NOT PROJECT_IS_TOP_LEVEL)
  message(
      AUTHOR_WARNING
      "Developer mode is intended for developers of glare"
  )
endif()

include(cmake/dev-mode.cmake)
