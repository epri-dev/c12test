cmake_minimum_required(VERSION 3.16)
set(EXECUTABLE_NAME "c12test")
add_executable(${EXECUTABLE_NAME} c12test.cpp Setup.cpp)
if (MSVC)
    # warning level 4 and all warnings as errors
    target_compile_options(${EXECUTABLE_NAME} PRIVATE "/WX")
else()
    # lots of warnings and all warnings as errors
    target_compile_options(${EXECUTABLE_NAME} PRIVATE "-Wall;-Wextra;-Wno-expansion-to-defined")
endif()
add_library(C12Tables STATIC C12Tables.cpp C12Meter.cpp)
target_compile_features(C12Tables PUBLIC cxx_std_17)
target_include_directories(C12Tables PRIVATE ${METERINGSDK_INCLUDE_DIR} ${METERINGSDK_BINARY_DIR})
target_compile_features(${EXECUTABLE_NAME} PUBLIC cxx_std_17)
target_include_directories(${EXECUTABLE_NAME} PRIVATE ${METERINGSDK_INCLUDE_DIR} ${METERINGSDK_BINARY_DIR})
target_link_libraries(${EXECUTABLE_NAME} PUBLIC MCOM MCORE PRIVATE C12Tables) 
install(TARGETS ${EXECUTABLE_NAME} DESTINATION bin)
