cmake_minimum_required(VERSION 3.16)
project(ravenxd_hud_desktop CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Linux (apt):   sudo apt install libsfml-dev
# Windows:       vcpkg install sfml
#                then pass -DCMAKE_TOOLCHAIN_FILE=<vcpkg>/scripts/buildsystems/vcpkg.cmake
find_package(SFML 2.5 COMPONENTS graphics window system REQUIRED)

add_executable(ravenxd_hud_desktop main.cpp)
target_link_libraries(ravenxd_hud_desktop PRIVATE sfml-graphics sfml-window sfml-system)

# Needs a threads library for the simulated background update thread.
find_package(Threads REQUIRED)
target_link_libraries(ravenxd_hud_desktop PRIVATE Threads::Threads)
