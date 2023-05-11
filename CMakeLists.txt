cmake_minimum_required(VERSION 3.0.0)
project(tp3 VERSION 0.1.0)

include(CTest)
enable_testing()

add_library(src block.cpp disqueVirtuel.cpp)
add_executable(tp3 main.cpp)

target_link_libraries(tp3 src)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
