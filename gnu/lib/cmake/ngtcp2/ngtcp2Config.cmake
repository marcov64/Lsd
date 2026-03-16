include(CMakeFindDependencyMacro)
if("TRUE")
    find_dependency(OpenSSL)
endif()

include("${CMAKE_CURRENT_LIST_DIR}/ngtcp2Targets.cmake")
