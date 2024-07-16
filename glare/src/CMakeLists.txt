set(GLARE_SCRIPT_CLING_ENABLED 0)
set(GLARE_SCRIPT_PRECOMPILED 1)
set(GLARE_USE_BOOST_PFR 1)
#set(GLARE_USE_BOOST_REFLECT 1)

include(FetchContent)

# Shared (core) utilities:
add_library(glare_core OBJECT) # STATIC # SHARED
add_library(${PROJECT_NAME}::core ALIAS glare_core)

target_compile_features(glare_core PRIVATE cxx_std_23)
set_property(TARGET glare_core PROPERTY CXX_STANDARD 23)
set_property(TARGET glare_core PROPERTY CXX_STANDARD_REQUIRED ON)

target_compile_definitions(glare_core PUBLIC GLARE_SCRIPT_PRECOMPILED=${GLARE_SCRIPT_PRECOMPILED})
target_compile_definitions(glare_core PUBLIC GLARE_SCRIPT_CLING_ENABLED=${GLARE_SCRIPT_CLING_ENABLED})

#set_target_properties(glare_core PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS 1)
#target_compile_definitions(glare_core PRIVATE GLARE_UTIL_EXPORT_SYMBOLS=1) # PUBLIC

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
    PUBLIC # PRIVATE
    $<TARGET_NAME_IF_EXISTS:SDL2::SDL2main>
    $<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>
)

find_package(concurrencpp CONFIG REQUIRED)
target_link_libraries(glare_core PUBLIC concurrencpp::concurrencpp) # PRIVATE

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

if (GLARE_USE_BOOST_PFR)
    #find_package(Boost CONFIG REQUIRED Boost::pfr)

    target_link_libraries(
        glare_core
        PUBLIC # PRIVATE
        Boost::pfr
    )
endif()

if (GLARE_USE_BOOST_REFLECT)
    #find_package(Boost CONFIG REQUIRED Boost::Reflect)

    target_link_libraries(
        glare_core
        PUBLIC # PRIVATE
        Boost::Reflect
    )
endif()

find_package(folly CONFIG REQUIRED) # find_package(folly CONFIG REQUIRED COMPONENTS small_vector)

# Remove just the "/std:c++17" portion:
#get_target_property(glare_folly_interface_compile_options Folly::folly INTERFACE_COMPILE_OPTIONS)
#list(REMOVE_ITEM glare_folly_interface_compile_options "/std:c++17")

# Original:
#set(glare_folly_interface_compile_options "/EHs;/GF;/Zc:referenceBinding;/Zc:rvalueCast;/Zc:implicitNoexcept;/Zc:strictStrings;/Zc:threadSafeInit;/Zc:throwingNew;/permissive-;/std:c++17;/wd4191;/wd4291;/wd4309;/wd4310;/wd4366;/wd4587;/wd4592;/wd4628;/wd4723;/wd4724;/wd4868;/wd4996;/wd4068;/wd4091;/wd4146;/wd4800;/wd4018;/wd4365;/wd4388;/wd4389;/wd4100;/wd4459;/wd4505;/wd4701;/wd4702;/wd4061;/wd4127;/wd4200;/wd4201;/wd4296;/wd4316;/wd4324;/wd4355;/wd4371;/wd4435;/wd4514;/wd4548;/wd4571;/wd4574;/wd4582;/wd4583;/wd4619;/wd4623;/wd4625;/wd4626;/wd4643;/wd4647;/wd4668;/wd4706;/wd4710;/wd4711;/wd4714;/wd4820;/wd5026;/wd5027;/wd5031;/wd5045;/we4099;/we4129;/we4566")

# Just warnings:
set(glare_folly_interface_compile_options "/wd4191;/wd4291;/wd4309;/wd4310;/wd4366;/wd4587;/wd4592;/wd4628;/wd4723;/wd4724;/wd4868;/wd4996;/wd4068;/wd4091;/wd4146;/wd4800;/wd4018;/wd4365;/wd4388;/wd4389;/wd4100;/wd4459;/wd4505;/wd4701;/wd4702;/wd4061;/wd4127;/wd4200;/wd4201;/wd4296;/wd4316;/wd4324;/wd4355;/wd4371;/wd4435;/wd4514;/wd4548;/wd4571;/wd4574;/wd4582;/wd4583;/wd4619;/wd4623;/wd4625;/wd4626;/wd4643;/wd4647;/wd4668;/wd4706;/wd4710;/wd4711;/wd4714;/wd4820;/wd5026;/wd5027;/wd5031;/wd5045;/we4099;/we4129;/we4566")

# Re-assign the target's compile options.
set_target_properties(Folly::folly PROPERTIES INTERFACE_COMPILE_OPTIONS "${glare_folly_interface_compile_options}")

target_link_libraries(glare_core PUBLIC Folly::folly) # Folly::folly_deps # PRIVATE

#find_package(Boost REQUIRED COMPONENTS boost_url)
#target_link_libraries(glare_core PUBLIC Boost::boost Boost::boost_url) # PRIVATE

find_package(fmt CONFIG REQUIRED)
target_link_libraries(glare_core PUBLIC fmt::fmt) # fmt::fmt-header-only # PRIVATE

find_package(nlohmann_json CONFIG REQUIRED)
target_link_libraries(glare_core PUBLIC nlohmann_json::nlohmann_json) # PRIVATE

find_package(magic_enum CONFIG REQUIRED)
target_link_libraries(glare_core PUBLIC magic_enum::magic_enum) # PRIVATE

find_package(spdlog CONFIG REQUIRED)
target_link_libraries(glare_core PUBLIC spdlog::spdlog_header_only) # spdlog::spdlog # PRIVATE

if (GLARE_USE_BOOST_PFR)
    #find_package(Boost CONFIG REQUIRED Boost::pfr)

    target_link_libraries(
        glare_core
        PUBLIC # PRIVATE
        Boost::pfr
    )
endif()

find_package(glm CONFIG REQUIRED)
target_link_libraries(glare_core PUBLIC glm::glm) # PRIVATE

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