set(GLARE_SCRIPT_CLING_ENABLED 0)
set(GLARE_SCRIPT_PRECOMPILED 1)
set(GLARE_USE_BOOST_PFR 1)
#set(GLARE_USE_BOOST_REFLECT 1)

include(FetchContent)

if (GLARE_USE_BOOST_PFR)
    FetchContent_Declare(
      boost_pfr
      GIT_REPOSITORY https://github.com/boostorg/pfr.git
      GIT_TAG        294a497 # release-2.2.0 (Boost 1.84)
      FIND_PACKAGE_ARGS NAMES Boost::pfr
    )

    FetchContent_MakeAvailable(boost_pfr)

    add_compile_definitions(GLARE_BOOST_PFR_ENABLED=1)
endif()

if (GLARE_USE_BOOST_REFLECT)
    FetchContent_Declare(
        boost_reflect
        GIT_REPOSITORY https://github.com/boost-ext/reflect.git
        GIT_TAG        34f04c4 # v1.0.8
        CONFIGURE_COMMAND ""
        BUILD_COMMAND ""
    )

    FetchContent_GetProperties(boost_reflect)

    if (NOT boost_reflect_POPULATED)
        FetchContent_Populate(boost_reflect)
    endif()

    add_library(boost_reflect INTERFACE)
    add_library(Boost::Reflect ALIAS boost_reflect)

    target_include_directories(boost_reflect INTERFACE ${boost_reflect_SOURCE_DIR})

    add_compile_definitions(GLARE_BOOST_REFLECT_ENABLED=1)
endif()


# Shared (core) utilities:
add_library(glare_core OBJECT) # STATIC # SHARED
add_library(${PROJECT_NAME}::core ALIAS glare_core)

target_compile_definitions(glare_core PUBLIC GLARE_SCRIPT_PRECOMPILED=${GLARE_SCRIPT_PRECOMPILED})
target_compile_definitions(glare_core PUBLIC GLARE_SCRIPT_CLING_ENABLED=${GLARE_SCRIPT_CLING_ENABLED})

#set_target_properties(glare_core PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS 1)
#target_compile_definitions(glare_core PRIVATE GLARE_UTIL_EXPORT_SYMBOLS=1)

target_compile_features(glare_core PRIVATE cxx_std_23)

# Propagates the top-level `src` directory to anyone using `core`.
target_include_directories(
    glare_core
    ${warning_guard}
    PUBLIC
    "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/glare/src>"
)

find_package(SDL2 CONFIG REQUIRED)
target_link_libraries(
    glare_core
    PRIVATE
    $<TARGET_NAME_IF_EXISTS:SDL2::SDL2main>
    $<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>
)

find_package(concurrencpp CONFIG REQUIRED)
target_link_libraries(glare_core PRIVATE concurrencpp::concurrencpp)

if (GLARE_USE_BOOST_PFR)
    #find_package(Boost CONFIG REQUIRED Boost::pfr)

    target_link_libraries(
        glare_core
        PUBLIC
        Boost::pfr
    )
endif()

if (GLARE_USE_BOOST_REFLECT)
    #find_package(Boost CONFIG REQUIRED Boost::Reflect)

    target_link_libraries(
        glare_core
        PUBLIC
        Boost::Reflect
    )
endif()

#set_target_properties(glare_core PROPERTIES UNITY_BUILD ON UNITY_BUILD_MODE BATCH UNITY_BUILD_BATCH_SIZE 32)

add_subdirectory("third-party")
add_subdirectory("util")
add_subdirectory("math")

# TODO: Rework into other modules, with remainder still in `core`.
add_subdirectory("app")

add_subdirectory("graphics")

add_subdirectory("engine")

add_subdirectory("script")

# TODO: Rework into other modules, with remainder still in `engine`.
add_subdirectory("game")