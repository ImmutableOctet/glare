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

# Engine source code
add_subdirectory("glare/src")

# ---- Declare executables ----

# Game
add_subdirectory("glare/project_glare")

# Scene Editor
add_subdirectory("glare/scene_editor")

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
