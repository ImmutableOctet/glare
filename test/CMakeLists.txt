project(glareTests LANGUAGES CXX)

# ---- Dependencies ----

find_package(Catch2 REQUIRED)
include(Catch)

# ---- Tests ----

add_executable(glare_test "src/engine/entity/serial.cpp" "src/engine/meta/meta.cpp" "src/util/string.cpp") # "src/util/parse.cpp"

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