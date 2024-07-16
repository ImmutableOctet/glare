if(GLARE_SCRIPT_CLING_ENABLED)
    set(GLARE_CLING_USE_THIRDPARTY ON)
endif()

#if(GLARE_SCRIPT_PRECOMPILED)
    #set(GLARE_SCRIPT_PRECOMPILED_GENERATE_CONSUMPTION_FILES ON)
    #set(GLARE_SCRIPT_PRECOMPILED_GENERATE_HEADER_FILES ON)
    ##set(GLARE_SCRIPT_PRECOMPILED_UNITY ON)
#endif()

#set(GLARE_RUNTIME_FILES ${CMAKE_SOURCE_DIR}/glare/glare)
set(GLARE_RUNTIME_ENGINE_FILES_BASE ${PROJECT_SOURCE_DIR})
set(GLARE_RUNTIME_ENGINE_FILES ${GLARE_RUNTIME_ENGINE_FILES_BASE}/engine)

include(fetch-cling.cmake)
include(script_generator.cmake)

if(GLARE_SCRIPT_CLING_ENABLED)
    add_library(glare_script OBJECT) # STATIC # SHARED

    #set_target_properties(
    #    glare_script
    #    PROPERTIES
    #    LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}
    #)
else()
    add_library(glare_script OBJECT) # STATIC
endif()

add_library(${PROJECT_NAME}::script ALIAS glare_script)

target_compile_features(glare_script PRIVATE cxx_std_23)

find_package(concurrencpp CONFIG REQUIRED)
target_link_libraries(glare_script PRIVATE concurrencpp::concurrencpp)

if(GLARE_SCRIPT_CLING_ENABLED)
    target_sources(
        glare_script
    
        PRIVATE
    
        "cling.cpp"
    )
endif()

glare_generate_cpp_script_bindings(glare_script "${GLARE_RUNTIME_ENGINE_FILES}" "${GLARE_RUNTIME_ENGINE_FILES_BASE}" "engine" 0)