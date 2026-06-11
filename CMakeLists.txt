cmake_minimum_required(VERSION 3.20)

# 1. Declarar un único proyecto global
project(opti_suite VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(MPI REQUIRED COMPONENTS CXX)

# 2. Definir el archivo que contiene el núcleo del optimizador (tu main + MPI)
# Asegúrate de ajustar este nombre si tu archivo central se llama diferente.
set(OPTIMIZER5D_CORE src/5d/opti5d.cpp)
set(OPTIMIZER2D_CORE src/2d/opti2d.cpp)

# 3. Lista de todas las funciones objetivo a compilar
set(OBJECTIVE_5D
    opti-mae
    opti-sae
    opti-sse
)

set(OBJECTIVE_2D
    camel
    michaelwicz
    rosenbrock
    sinoidal
)

# 4. Bucle para generar un ejecutable por cada función objetivo
foreach(func IN LISTS OBJECTIVE_5D)
    # Compila el ejecutable uniendo el optimizador base con la función específica
    add_executable(${func} ${OPTIMIZER5D_CORE} src/5d/${func}.cpp)
    
    # Enlaza la librería de MPI al ejecutable recién creado
    target_link_libraries(${func} PRIVATE MPI::MPI_CXX)
endforeach()

foreach(func IN LISTS OBJECTIVE_2D)
    add_executable(${func} ${OPTIMIZER2D_CORE} src/2d/${func}.cpp)
    target_link_libraries(${func} PRIVATE MPI::MPI_CXX)
    
endforeach(func IN LISTS OBJECTIVE_2D)


# Opcional: Si 'opti' es un ejecutable que no sigue el mismo patrón, 
# se puede compilar por separado fuera del bucle:
# add_executable(opti src/opti.cpp)
# target_link_libraries(opti PRIVATE MPI::MPI_CXX)