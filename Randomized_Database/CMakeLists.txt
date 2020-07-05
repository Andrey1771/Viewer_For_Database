cmake_minimum_required(VERSION 2.8.12)
project(Randomized_Database)

find_package(Qt5 COMPONENTS Core Widgets Sql REQUIRED)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

file(GLOB_RECURSE HEADERS LIST_DIRECTORIES false *.h)
file(GLOB_RECURSE SOURCES LIST_DIRECTORIES false *.cpp)
file(GLOB_RECURSE UIS LIST_DIRECTORIES false *.ui)

add_executable(${PROJECT_NAME} WIN32 ${HEADERS} ${SOURCES} ${UIS} ${UICS})

target_link_libraries(${PROJECT_NAME} PUBLIC Qt5::Core Qt5::Widgets Qt5::Sql)

#include_directories(../TestLoggerLib)
#target_link_libraries(${PROJECT_NAME} PUBLIC TestLoggerLib)
