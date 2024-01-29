include(FetchContent)

FetchContent_Declare(
  boost_pfr
  GIT_REPOSITORY https://github.com/boostorg/pfr.git
  GIT_TAG        294a497 # release-2.2.0 (Boost 1.84)
  FIND_PACKAGE_ARGS NAMES Boost::pfr
)

FetchContent_MakeAvailable(boost_pfr)

# Shared (core) utilities:
add_library(glare_core OBJECT) # STATIC
add_library(${PROJECT_NAME}::core ALIAS glare_core)

target_compile_features(glare_core PRIVATE cxx_std_23)

target_sources(
    glare_core
    
    PRIVATE
    
    "lib.cpp"
)

# Propagates the top-level `src` directory to anyone using `core`.
target_include_directories(
    glare_core
    ${warning_guard}
    PUBLIC
    "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/glare/src>"
)

find_package(SDL2 CONFIG REQUIRED)
target_link_libraries(glare_core
    PRIVATE
    $<TARGET_NAME_IF_EXISTS:SDL2::SDL2main>
    $<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>
)

#find_package(Boost CONFIG REQUIRED Boost::pfr)

target_link_libraries(glare_core
    PUBLIC
    Boost::pfr
)

add_subdirectory("third-party")
add_subdirectory("util")
add_subdirectory("math")

# TODO: Rework into other modules, with remainder still in `core`.
add_subdirectory("app")


# Shared graphics library:
add_library(glare_graphics OBJECT) # STATIC
add_library(${PROJECT_NAME}::graphics ALIAS glare_graphics)

target_compile_features(glare_graphics PRIVATE cxx_std_23)

find_package(SDL2 CONFIG REQUIRED)
target_link_libraries(glare_graphics
    PRIVATE # PUBLIC
    $<TARGET_NAME_IF_EXISTS:SDL2::SDL2main>
    $<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>
)

add_subdirectory("graphics")


# Game engine:
add_library(glare_engine OBJECT) # STATIC
add_library(${PROJECT_NAME}::engine ALIAS glare_engine)

target_compile_features(glare_engine PRIVATE cxx_std_23)

find_package(SDL2 CONFIG REQUIRED)
target_link_libraries(glare_engine
    PRIVATE # PUBLIC
    $<TARGET_NAME_IF_EXISTS:SDL2::SDL2main>
    $<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>
)

add_subdirectory("engine")

# TODO: Rework into other modules, with remainder still in `engine`.
add_subdirectory("game")

# Main library:
#add_library(glare STATIC) # OBJECT
#add_library(${PROJECT_NAME}::glare ALIAS glare)
#target_compile_features(glare PRIVATE cxx_std_23)