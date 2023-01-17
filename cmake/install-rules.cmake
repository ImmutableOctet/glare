install(
    TARGETS glare # glare_exe
    RUNTIME COMPONENT glare_Runtime
)

if(PROJECT_IS_TOP_LEVEL)
  include(CPack)
endif()
