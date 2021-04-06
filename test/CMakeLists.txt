cmake_minimum_required(VERSION 3.16)
include_directories( ${CMAKE_HOME_DIRECTORY}/src )
find_package(Threads)
find_package(GTest)
add_executable(C12TableTest C12TableTest.cpp)

target_link_libraries(C12TableTest C12Tables ${GTEST_BOTH_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
add_test(C12TableTests C12TableTest)
