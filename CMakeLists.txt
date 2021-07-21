cmake_minimum_required(VERSION 3.16)
if (POLICY CMP0048)
  cmake_policy(SET CMP0048 NEW)
  set(CMAKE_POLICY_DEFAULT_CMP0048 NEW)
endif (POLICY CMP0048)
if (POLICY CMP0077)
  cmake_policy(SET CMP0077 NEW)
  set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)
endif (POLICY CMP0077)

project(c12test VERSION 1.0.4.6 LANGUAGES C CXX)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/CMake/Modules/")
include(FetchContent)
FetchContent_Declare(C12Adapter 
    GIT_REPOSITORY https://github.com/beroset/C12Adapter.git
)

# Check if population has already been performed
FetchContent_GetProperties(C12Adapter)
string(TOLOWER "C12Adapter" lcName)
if(NOT ${lcName}_POPULATED)
    # Fetch the content using previously declared details
    FetchContent_Populate(C12Adapter)

    option(BUILD_EXAMPLES OFF)
    set(M_PRODUCT_NAME "c12test")
    set(M_COMPANY_NAME "EPRI")
    set(M_PRODUCT_LEGAL_COPYRIGHT "Copyright (c) 2021 EPRI")
    set(M_PRODUCT_VERSION    ${PROJECT_VERSION})
    set(M_NO_AUTOMATION 1)
    set(METERINGSDK_INCLUDE_DIR "${${lcName}_SOURCE_DIR}/src/MeteringSDK")
    include_directories("${${lcName}_BINARY_DIR}/MeteringSDK")
    # put libraries where we can find them and link to them
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")

  # Bring the populated content into the build
    add_subdirectory(${${lcName}_SOURCE_DIR} ${${lcName}_BINARY_DIR})
endif()

# options off-by-default that you can enable
option(BUILD_DOCS "Build the documentation" OFF)
option(BUILD_TESTING "Build test suite" OFF)

# options on-by-default that you can disable
option(BUILD_SHARED_LIBS "Build shared libraries" ON)


if (BUILD_DOCS)
    add_subdirectory(doc)
endif (BUILD_DOCS)

if (BUILD_TESTING)
    include(CTest)
    add_subdirectory(test)
endif() 

add_subdirectory(src)

set(CPACK_SOURCE_IGNORE_FILES "/build/;.swp;.git")
set(CPACK_PACKAGE_VENDOR "EPRI")
set(CPACK_SOURCE_GENERATOR "TGZ")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE")
include(InstallRequiredSystemLibraries)
include (CPack)
