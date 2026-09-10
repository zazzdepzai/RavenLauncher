cmake_minimum_required(VERSION 3.16)
project(MCLauncherCore CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(CURL REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(nlohmann_json REQUIRED)

add_library(mclauncher_core STATIC
    core/Manifest.cpp
    core/DownloadManager.cpp
    core/UpdateManager.cpp
    core/GameLauncher.cpp
)

target_include_directories(mclauncher_core PUBLIC core)
target_link_libraries(mclauncher_core PUBLIC
    CURL::libcurl
    OpenSSL::Crypto
    nlohmann_json::nlohmann_json
)

# The ios/ directory (Objective-C++) is NOT built here -- it's compiled as
# part of the Xcode app target, which links against libmclauncher_core.a.
# Add ios/*.mm and ios/*.h to your Xcode target's sources directly.
