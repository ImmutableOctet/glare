cmake_minimum_required(VERSION 3.28)

include(cmake/prelude.cmake)

include(cmake/project-is-top-level.cmake)
include(cmake/variables.cmake)

include(cmake/reflection.cmake)
include(cmake/gather_shared_sources.cmake)
include(cmake/ignore_list.cmake)
include(cmake/unity-build.cmake)

#set(CMAKE_BUILD_PARALLEL_LEVEL, 4)

#set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS, "ON")
#set(GLARE_BUILD_SHARED, "OFF")

if(GLARE_USE_CLANGCL)
    set(CMAKE_GENERATOR_TOOLSET "ClangCL")
endif()

project(
    glare
    VERSION 0.2.0
    DESCRIPTION "Glare - Open Source Game Engine"
    HOMEPAGE_URL "https://github.com/ImmutableOctet/glare"
    LANGUAGES CXX
)

# TODO: Revisit when we start using C++20 modules.
# Disabled for now.
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)

#if($<CONFIG:Debug>)
    if (MSVC)
        add_compile_options(/ZI)
        add_link_options(/DEBUG:FASTLINK /INCREMENTAL)
    endif()

    add_compile_definitions(GLARE_DEBUG=1)
#endif()

# Glare Engine source code.
# TODO: Remove to `src`, rather than `glare/src` subdirectory.
add_subdirectory("glare/src")

# ---- Post-build operations ----
include(cmake/post-build.cmake)

# ---- Install rules ----

#if(NOT CMAKE_SKIP_INSTALL_RULES)
#  include(cmake/install-rules.cmake)
#endif()

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
