cmake_minimum_required(VERSION 3.12)
project(interference_classification)

set(CMAKE_CXX_STANDARD 14)

add_executable(${PROJECT_NAME} main.cpp)

include_directories(${PROJECT_NAME} ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(${PROJECT_NAME} interference_static)

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${CMAKE_SOURCE_DIR}/dist/bin/classification)
