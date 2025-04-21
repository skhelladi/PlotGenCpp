cmake_minimum_required(VERSION 3.10)

# Nom du projet
project(PlotGenCpp)

# Définir le standard C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_BUILD_TYPE Release)
# set(CMAKE_BUILD_TYPE Debug)

# Trouver SFML
find_package(SFML 2.5 COMPONENTS graphics window system REQUIRED)

# Ajouter les fichiers sources pour le programme principal
set(SOURCES
    src/plotgen.cpp
)

include_directories(${PROJECT_SOURCE_DIR}/include)

# # Créer l'exécutable principal
# add_executable(${PROJECT_NAME} ${SOURCES})

# Créer librairie
add_library(${PROJECT_NAME} SHARED ${SOURCES})

# Ajouter l'exécutable pour les exemples
add_executable(PlotterExamples src/examples.cpp)

# Inclure les répertoires pour les en-têtes
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_include_directories(PlotterExamples PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Lier les bibliothèques SFML pour les deux exécutables
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

# Copier la police arial.ttf dans le répertoire de construction
# configure_file(arial.ttf arial.ttf COPYONLY)

# Installation (optionnel)
install(TARGETS ${PROJECT_NAME} PlotterExamples DESTINATION bin)
install(FILES arial.ttf DESTINATION bin)