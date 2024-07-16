project(glareTests LANGUAGES CXX)

set(GLARE_USE_BOOST_PFR 1)
#set(GLARE_USE_BOOST_REFLECT 1)

if (GLARE_USE_BOOST_PFR)
    add_compile_definitions(GLARE_BOOST_PFR_ENABLED=1)
endif()

# ---- Dependencies ----

#find_package(Catch2 REQUIRED)

include(FetchContent)

FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        v3.4.0 # or a later release
)

FetchContent_MakeAvailable(Catch2)

include(Catch)

# ---- Tests ----

add_executable(
    glare_test

    "src/engine/meta/meta.cpp"
    "src/engine/meta/serial.cpp"
    "src/engine/meta/variant_wrapper.cpp"
    "src/engine/entity/serial.cpp"
    "src/engine/entity/parse.cpp"
    "src/engine/meta/reflection_test.cpp"
    "src/engine/meta/meta_type_descriptor.cpp"
    
    "src/util/string.cpp"
    "src/util/parse.cpp"
    "src/util/algorithm.cpp"
    "src/util/sampler.cpp"
    "src/util/history_log.cpp"
    
    "src/math/conversion.cpp"
    "src/util/vector_queue.cpp"
    "src/game/game_stub.cpp"
)

target_compile_features(glare_test PRIVATE cxx_std_23)

target_link_libraries(
    glare_test
    
    PRIVATE
    
    # glare
    glare_app
    glare_engine
    glare_engine_reflection
    glare_core
    glare_graphics
    glare_script

    Catch2::Catch2WithMain
)

if (GLARE_SCRIPT_USE_CLING)
    add_custom_command(
        TARGET glare_test POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_RUNTIME_DLLS:glare_test> $<TARGET_FILE_DIR:glare_test>
        COMMAND_EXPAND_LISTS
    )
endif()

catch_discover_tests(glare_test)

# ---- End-of-file commands ----

add_folders(Test)
