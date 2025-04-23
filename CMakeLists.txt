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

# Fetch Simple SVG library but configure it manually to avoid GTest requirement
# include(FetchContent)
# FetchContent_Declare(
#     simple_svg
#     GIT_REPOSITORY https://github.com/adishavit/simple-svg.git
#     GIT_TAG master
# )
# # Just download the content but don't configure it yet
# FetchContent_GetProperties(simple_svg)
# if(NOT simple_svg_POPULATED)
#     FetchContent_Populate(simple_svg)
#     # Don't use FetchContent_MakeAvailable as we want to skip the original CMakeLists.txt
# endif()

# Simple SVG is a header-only library, so we can just include it
# Make sure to include the include directory, not the root directory
include_directories(${simple_svg_SOURCE_DIR}/include)

# Add source files for the main program
set(SOURCES
    src/plotgen.cpp
)

include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_SOURCE_DIR}/stb)

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

# Check for GTK3 and WEBKIT packages
find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
    pkg_check_modules(GTK3 QUIET gtk+-3.0)
    pkg_check_modules(WEBKIT QUIET webkit2gtk-4.1)
    
    # Check if both GTK3 and WEBKIT are found and properly configured
    if(GTK3_FOUND AND WEBKIT_FOUND AND GTK3_INCLUDE_DIRS AND WEBKIT_INCLUDE_DIRS)
        # Define preprocessor macro to enable GTK/WEBKIT features
        add_definitions(-DHAVE_GTK_WEBKIT)
        
        # Include the headers
        include_directories(${GTK3_INCLUDE_DIRS} ${WEBKIT_INCLUDE_DIRS})
        
        # Link the libraries to the main library
        target_link_libraries(${PROJECT_NAME} PRIVATE
            sfml-graphics
            sfml-window
            sfml-system
            ${GTK3_LIBRARIES}
            ${WEBKIT_LIBRARIES}
        )
        
        message(STATUS "GTK3 and WebKit found - HTMLViewer will be enabled")
    else()
        # WebKit or GTK3 not found, don't define HAVE_GTK_WEBKIT
        target_link_libraries(${PROJECT_NAME} PRIVATE
            sfml-graphics
            sfml-window
            sfml-system
        )
        message(STATUS "GTK3 and/or WebKit not found - HTMLViewer will be disabled")
    endif()
else()
    # pkg-config not found, don't define HAVE_GTK_WEBKIT
    target_link_libraries(${PROJECT_NAME} PRIVATE
        sfml-graphics
        sfml-window
        sfml-system
    )
    message(STATUS "pkg-config not found - HTMLViewer will be disabled")
endif()

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

# Installation (optional)
install(TARGETS ${PROJECT_NAME} PlotterExamples DESTINATION bin)
install(FILES arial.ttf DESTINATION bin)
