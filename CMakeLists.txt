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
# Don't build the examples within C12Adapter
option(BUILD_EXAMPLES OFF)

# options on-by-default that you can disable
option(BUILD_SHARED_LIBS "Build shared libraries" ON)


# configure C12Adapter 
set(M_PRODUCT_NAME "c12test")
set(M_COMPANY_NAME "EPRI")
set(M_PRODUCT_LEGAL_COPYRIGHT "EPRI")
set(M_PRODUCT_VERSION_MAJOR    ${PRODUCT_VERSION_MAJOR})
set(M_PRODUCT_VERSION_MIDDLE   ${PRODUCT_VERSION_MIDDLE})
set(M_PRODUCT_VERSION_MINOR    ${PRODUCT_VERSION_MINOR})
set(M_PRODUCT_VERSION_TAG      ${PRODUCT_VERSION_TAG})

# put libraries where we can find them and link to them
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(METERINGSDK_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/C12Adapter/src/MeteringSDK")
add_subdirectory(C12Adapter)

set(METERINGSDK_BINARY_DIR "${CMAKE_BINARY_DIR}/C12Adapter/MeteringSDK")
add_subdirectory(src)
if (BUILD_DOCS)
    add_subdirectory(doc)
endif (BUILD_DOCS)

if (BUILD_TESTING)
    include(CTest)
    add_subdirectory(test)
endif() 

set(CPACK_SOURCE_IGNORE_FILES "/build/;.swp;.git")
set(CPACK_PACKAGE_VENDOR "EPRI")
set(CPACK_SOURCE_GENERATOR "TGZ")
include(InstallRequiredSystemLibraries)
include (CPack)
