cmake_minimum_required(VERSION 2.8.12)
project(Results_Table_View)

find_package(Qt5 COMPONENTS Core Widgets Sql PrintSupport Test REQUIRED)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

file(GLOB_RECURSE HEADERS LIST_DIRECTORIES false *.h)
file(GLOB_RECURSE SOURCES LIST_DIRECTORIES false *.cpp)
file(GLOB_RECURSE UIS LIST_DIRECTORIES false *.ui)
file(GLOB_RECURSE RESOURCE LIST_DIRECTORIES false *.qrc)

add_executable(${PROJECT_NAME} WIN32 ${HEADERS} ${SOURCES} ${RESOURCE} ${UIS} ${UICS})
target_link_libraries(${PROJECT_NAME} PUBLIC Qt5::Core Qt5::Widgets Qt5::Sql Qt5::PrintSupport Qt5::Test)
#target_link_libraries(${PROJECT_NAME} PUBLIC InstrumentUtilsLib)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
#    ${CMAKE_CURRENT_SOURCE_DIR}InstrumentUtilsLib
    )
#set(SOURCES InstrumentUtilsLib.cpp)
#set(HEADERS InstrumentUtilsLib.h)
#add_library(InstrumentUtilsLib ${SOURCES} ${HEADERS})
#target_include_directories(InstrumentUtilsLib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/InstrumentUtilsLib)
#include_directories(../InstrumentUtilsLib)
