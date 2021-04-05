cmake_minimum_required(VERSION 3.16)
if (POLICY CMP0048)
  cmake_policy(SET CMP0048 NEW)
  set(CMAKE_POLICY_DEFAULT_CMP0048 NEW)
endif (POLICY CMP0048)
if (POLICY CMP0077)
  cmake_policy(SET CMP0077 NEW)
  set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)
endif (POLICY CMP0077)

project(c12test
    VERSION 1.0.0)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/CMake/Modules/")

# options off-by-default that you can enable
option(BUILD_DOCS "Build the documentation" OFF)

# options on-by-default that you can disable
option(BUILD_SHARED_LIBS "Build shared libraries" ON)

# Don't build the examples within C12Adapter
option(BUILD_EXAMPLES OFF)
set(METERINGSDK_INCLUDE_DIR "C12Adapter/src/MeteringSDK")
add_subdirectory(C12Adapter)

set(CMAKE_CXX_STANDARD 17)
add_subdirectory(src)
if (BUILD_DOCS)
    add_subdirectory(doc)
endif (BUILD_DOCS)
SET(CPACK_SOURCE_IGNORE_FILES "/build/;.swp;.git")
SET(CPACK_PACKAGE_VENDOR "EPRI")
set(CPACK_SOURCE_GENERATOR "TGZ")
INCLUDE(InstallRequiredSystemLibraries)
include (CPack)
