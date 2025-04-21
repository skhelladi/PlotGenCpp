cmake_minimum_required(VERSION 3.10)

# Project name
project(PlotGenCpp VERSION 0.1 LANGUAGES CXX)

# Define C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_BUILD_TYPE Release)
# set(CMAKE_BUILD_TYPE Debug)

# Find SFML
find_package(SFML 2.5 COMPONENTS graphics window system REQUIRED)

# Add source files for the main program
set(SOURCES
    src/plotgen.cpp
)

include_directories(${PROJECT_SOURCE_DIR}/include)

# # Create the main executable
# add_executable(${PROJECT_NAME} ${SOURCES})

# Create library
add_library(${PROJECT_NAME} SHARED ${SOURCES})

# Add executable for examples
add_executable(PlotterExamples src/examples.cpp)

# Include header directories
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_include_directories(PlotterExamples PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Link SFML libraries for both executables
target_link_libraries(${PROJECT_NAME} PRIVATE
    sfml-graphics
    sfml-window
    sfml-system
)

target_link_libraries(PlotterExamples PRIVATE
    sfml-graphics
    sfml-window
    sfml-system
    ${PROJECT_NAME}
)

# Add compiler flags for optimization and warnings
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(${PROJECT_NAME} PRIVATE
        -O3
        -Wall
        # -Wextra
        # -Wpedantic
        # -Wno-unused-parameter
    )
elseif(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE
        /O2
        /W4
    )
endif()

# Copy arial.ttf font to the build directory
# configure_file(arial.ttf arial.ttf COPYONLY)

# Installation (optional)
install(TARGETS ${PROJECT_NAME} PlotterExamples DESTINATION bin)
install(FILES arial.ttf DESTINATION bin)
