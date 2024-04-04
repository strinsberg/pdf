cmake_minimum_required(VERSION 3.16)

project(pdfcli VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# GTest as a depedency as recommended by google test for cmake
include(FetchContent)
FetchContent_Declare(
  googletest
  # Specify the commit you depend on and update it regularly.
  URL https://github.com/google/googletest/archive/5376968f6948923e2411081fd9372e71a59d8e77.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Add the source files
file(GLOB_RECURSE PDFCLI_SOURCES src/*.cpp)
list(REMOVE_ITEM PDFCLI_SOURCES main.cpp)

file(GLOB_RECURSE PDFCLI_INCLUDES include/*.h)

file(GLOB_RECURSE PDFCLI_TESTS test/*.cpp)

# Add the sources as a lib
add_library(${PROJECT_NAME}_lib STATIC ${PDFCLI_SOURCES} ${PDFCLI_INCLUDES})

# Add include directories
# cmake says to use target_include_directories but I was having trouble with
# headers not being found in main.cpp for the setup that uses a static lib
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# Add main executable
add_executable(${PROJECT_NAME} "src/main.cpp")
target_link_libraries(${PROJECT_NAME} ${PROJECT_NAME}_lib)
target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Werror -Wpedantic -Wextra)
set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Add testing
enable_testing()
add_executable(test_${PROJECT_NAME} ${PDFCLI_TESTS})
target_link_libraries(test_${PROJECT_NAME} ${PROJECT_NAME}_lib)
target_link_libraries(test_${PROJECT_NAME} gtest gmock gtest_main)
set_target_properties(test_${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
add_test(NAME ${PROJECT_NAME} COMMAND test_${PROJECT_NAME})
include(GoogleTest)
