if (GLARE_SCRIPT_CLING_ENABLED)
    if (GLARE_CLING_USE_THIRDPARTY)
            #set(LLVM_TARGETS_TO_BUILD "host;NVPTX")

            # FetchContent:
            #FetchContent_Declare(
            #  cling
            #  SOURCE_DIR "${PROJECT_SOURCE_DIR}/thirdparty/cling/cling/"
            #  FIND_PACKAGE_ARGS NAMES cling
            #)
            #
            #FetchContent_MakeAvailable(cling)
    
            #find_library(cling
            #    cling clingInterpreter
            #    HINTS "${CMAKE_PREFIX_PATH}/third-party"
            #)

            # Direct usage:
            set(CLING_DIR "${PROJECT_SOURCE_DIR}/third-party/cling")
            set(CLING_PREFIX "${PROJECT_SOURCE_DIR}/third-party/cling/cling")
            set(LLVM_BUILD_BINARY_DIR "${PROJECT_SOURCE_DIR}/third-party/cling/build")
            set(CLING_CLANG_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/third-party/cling/src/clang/include")

            include("${PROJECT_SOURCE_DIR}/third-party/cmake/FindCling.cmake")
            include("${PROJECT_SOURCE_DIR}/third-party/cmake/FindClangForCling.cmake")
        else()
            FetchContent_Declare(
              cling
              GIT_REPOSITORY https://github.com/root-project/cling.git
              GIT_TAG        ab81cdc # v1.0
              FIND_PACKAGE_ARGS NAMES cling
            )

            FetchContent_MakeAvailable(cling)
        endif()
endif()