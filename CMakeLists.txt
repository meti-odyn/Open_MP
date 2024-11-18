cmake_minimum_required(VERSION 3.29)
project(Open_MP)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp -v -Wall")
#add_compile_options(-fopenmp -Wall)

#bierze ten plik jako główny który ma funkcje int main()
add_executable(Open_MP main.cpp
        inflection_map.cpp
        sequence.cpp
        sequence_vector.cpp)
