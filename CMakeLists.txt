cmake_minimum_required(VERSION 3.21)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)

project(RTOneWeekend)

file(GLOB H_SOURCES "src/*.h")
file(GLOB CPP_SOURCES "src/*.cpp")
file(GLOB C_SOURCES "src/*.c")
file(GLOB SUBFOLDER_H_SOURCES "src/*/*.h")
file(GLOB SUBFOLDER_CPP_SOURCES "src/*/*.cpp")
file(GLOB SUBFOLDER_C_SOURCES "src/*/*.c")

add_executable(RTOneWeekend ${C_SOURCES} ${CPP_SOURCES} ${H_SOURCES} ${SUBFOLDER_H_SOURCES} ${SUBFOLDER_C_SOURCES} ${SUBFOLDER_CPP_SOURCES})

target_compile_options(RTOneWeekend PRIVATE -pthread)
set_target_properties(RTOneWeekend PROPERTIES OUTPUT_NAME "RTOneWeekend")

add_subdirectory(libraries/glm)

add_subdirectory(libraries/spdlog)

target_link_libraries(RTOneWeekend glm spdlog::spdlog)