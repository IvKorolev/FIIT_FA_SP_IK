cmake_minimum_required(VERSION 3.25 FATAL_ERROR)

project(mp_os LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

message([[VCPKG toolchain setup example. It is needed for some libraries. Highly recommended to pass it by command line arguments for cmake configuration (IDE can help with this).
        set(VCPKG_TARGET_TRIPLET x64-windows)
        set(CMAKE_TOOLCHAIN_FILE C:/Programming/vcpkg/scripts/buildsystems/vcpkg.cmake)]])

include(FetchContent)
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip)
FetchContent_MakeAvailable(googletest)

FetchContent_Declare(
        nlohmann_json
        GIT_REPOSITORY https://github.com/nlohmann/json.git
        GIT_TAG v3.11.3
)
FetchContent_MakeAvailable(nlohmann_json)

set(VCPKG_ROOT "D:/ClionProjects/FIIT_FA_SP_IK/vcpkg")
set(VCPKG_INCLUDE_DIR "${VCPKG_ROOT}/installed/x64-windows/include")

set(BOOST_ROOT "D:/ClionProjects/FIIT_FA_SP_IK/vcpkg/installed/x64-windows")  # Укажите правильный путь
set(BOOST_INCLUDEDIR "${BOOST_ROOT}/include")
set(BOOST_LIBRARYDIR "${BOOST_ROOT}/lib")

find_package(Crow CONFIG REQUIRED)
find_package(Asio CONFIG REQUIRED)
find_package(httplib CONFIG REQUIRED)

add_subdirectory(allocator)
add_subdirectory(arithmetic)
add_subdirectory(associative_container)
add_subdirectory(common)
add_subdirectory(logger)