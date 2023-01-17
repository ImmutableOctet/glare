#add_library(glare_util OBJECT) # STATIC

target_sources(
    glare_core # glare_util
    
    PRIVATE
    
    "io.cpp"
    "json.cpp"
    "string.cpp"
    "parse.cpp"
)

find_package(folly CONFIG REQUIRED) # find_package(folly CONFIG REQUIRED COMPONENTS small_vector)
target_link_libraries(glare_core PUBLIC Folly::folly) # Folly::folly_deps # glare_common

#find_package(Boost REQUIRED COMPONENTS boost_url)
#target_link_libraries(glare_core PUBLIC Boost::boost Boost::boost_url) # glare_common

find_package(fmt CONFIG REQUIRED)
target_link_libraries(glare_core PUBLIC fmt::fmt) # fmt::fmt-header-only # glare_common

find_package(nlohmann_json CONFIG REQUIRED)
target_link_libraries(glare_core PUBLIC nlohmann_json::nlohmann_json) # glare_common

find_package(magic_enum CONFIG REQUIRED)
target_link_libraries(glare_core PUBLIC magic_enum::magic_enum) # glare_common

find_package(spdlog CONFIG REQUIRED)
target_link_libraries(glare_core PUBLIC spdlog::spdlog_header_only) # spdlog::spdlog # glare_common

#add_dependencies(
#    glare_util
#    glare_common
#)
#
#target_link_libraries(
#    glare_util
#    
#    PRIVATE
#
#    glare_common
#)