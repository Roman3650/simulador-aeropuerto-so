cmake_minimum_required(VERSION 3.16)

project(SimuladorAeropuerto LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Threads REQUIRED)

# Equivale a "src/*.cpp", pero CMake vuelve a configurar
# automaticamente cuando se agrega o quita un .cpp.
file(GLOB FUENTES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

add_executable(aeropuerto ${FUENTES})

target_include_directories(aeropuerto PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_link_libraries(aeropuerto PRIVATE Threads::Threads)
