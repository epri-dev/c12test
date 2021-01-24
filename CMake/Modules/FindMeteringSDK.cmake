set(_x86 "(x86)")
file(GLOB _MeteringSDK_INCLUDE_DIRS
    "$ENV{ProgramFiles}/MeteringSDK/*/include"
    "$ENV{ProgramFiles${_x86}}/MeteringSDK/*/include"
    )
unset(_x86)

find_path(METERINGSDK_INCLUDE_DIR
        NAMES
        MCORE/MCORE.h
        PATHS
        ${_MeteringSDK_INCLUDE_DIRS}
        /usr/include/MeteringSDK
        /usr/local/include/MeteringSDK
        ${MCORE_ROOT}
)

if(METERINGSDK_INCLUDE_DIR AND EXISTS "${METERINGSDK_INCLUDE_DIR}/MCORE/MeteringSDKVersion.h")
    file(STRINGS "${METERINGSDK_INCLUDE_DIR}/MCORE/MeteringSDKVersion.h" _contents REGEX "^#define M_SDK_VERSION_.+[ \t]+[0-9]+")
    string(REGEX REPLACE ".*VERSION_MAJOR[ \t]+([0-9]+).*" "\\1" MCORE_VERSION_MAJOR "${_contents}")
    string(REGEX REPLACE ".*VERSION_MIDDLE[ \t]+([0-9]+).*" "\\1" MCORE_VERSION_MINOR "${_contents}")
    string(REGEX REPLACE ".*VERSION_MINOR[ \t]+([0-9]+).*" "\\1" MCORE_VERSION_PATCH "${_contents}")
    set(MCORE_VERSION "${MCORE_VERSION_MAJOR}.${MCORE_VERSION_MINOR}.${MCORE_VERSION_PATCH}")
    message(STATUS "Found MeteringSDK version ${MCORE_VERSION}")
endif()

find_library(METERINGSDK_MCORE_LIBS
    NAMES MCORE)
find_library(METERINGSDK_MCOM_LIBS
    NAMES MCOM)
set(METERINGSDK_LIBS "${METERINGSDK_MCORE_LIBS}" "${METERINGSDK_MCOM_LIBS}")

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
    MeteringSDK
    DEFAULT_MSG
    METERINGSDK_INCLUDE_DIR
)
