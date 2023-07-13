project(glareTests LANGUAGES CXX)

# ---- Dependencies ----

find_package(Catch2 REQUIRED)
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
)

target_compile_features(glare_test PRIVATE cxx_std_23)

target_link_libraries(
    glare_test
    
    PRIVATE
    
    # glare
    glare_core
    glare_graphics
    glare_engine

    Catch2::Catch2WithMain
)

catch_discover_tests(glare_test)

# ---- End-of-file commands ----

add_folders(Test)
