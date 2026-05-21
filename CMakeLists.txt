cmake_minimum_required(VERSION 3.10.0)
project(RealTimeSimulation VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_compile_options(-Wall -Wextra -march=native)

option(GENERATE_ASM "Gera arquivos .s com assembly comentado durante o build" ON)
if(GENERATE_ASM)
    # -save-temps=obj: Salva os arquivos .s na pasta de build junto com os .o
    # -fverbose-asm: Adiciona comentários no assembly para facilitar a leitura
    # -g: Inclui informações de debug para que ferramentas (ou você) mapeiem ASM para C++
    add_compile_options(-save-temps=obj -fverbose-asm -g -fopt-info-vec-optimized)
    set(CMAKE_DISABLE_PRECOMPILE_HEADERS ON)
    message(STATUS "Assembly ON: PCH desativado para evitar conflitos.")
endif()

add_executable(simulation src/main.cpp)
target_precompile_headers(simulation PRIVATE src/Types.h)
target_include_directories(simulation PUBLIC src)

add_executable(simulation_tests tests/simulation_methods.cpp)
target_precompile_headers(simulation_tests PRIVATE src/Types.h)
target_precompile_headers(simulation_tests PRIVATE tests/Test.h)
target_include_directories(simulation_tests PRIVATE src)

add_executable(simulation_results_tests tests/simulation_results.cpp)
target_precompile_headers(simulation_results_tests PRIVATE src/Types.h)
target_precompile_headers(simulation_results_tests PRIVATE tests/Test.h)
target_include_directories(simulation_results_tests PRIVATE src)

