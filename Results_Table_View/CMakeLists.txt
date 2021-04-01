cmake_minimum_required(VERSION 2.8.12)
project(Results_Table_View)

find_package(Qt5 COMPONENTS Core Widgets Sql PrintSupport Test LinguistTools REQUIRED)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(APP_ICON_RESOURCE_WINDOWS "${CMAKE_CURRENT_SOURCE_DIR}/myapp.rc")

file(GLOB_RECURSE HEADERS LIST_DIRECTORIES false *.h)
file(GLOB_RECURSE SOURCES LIST_DIRECTORIES false *.cpp)
file(GLOB_RECURSE UIS LIST_DIRECTORIES false *.ui)
file(GLOB_RECURSE RESOURCE LIST_DIRECTORIES false *.qrc)


include(${CMAKE_SOURCE_DIR}/projects/translations_helper.cmake)
set(LANGUAGES

    ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}_en.ts
    ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}_ru.ts)

create_and_update_translations(TRANSLATIONS ${HEADERS} ${SOURCES} ${UIS} ${RESOURCE} ${LANGUAGES} OPTIONS -locations none -silent ${LUPDATE_ADDITIONAL_OPTION})

add_executable(${PROJECT_NAME} WIN32 ${HEADERS}
    ${SOURCES}
    ${RESOURCE}
    ${UIS}
    ${UICS}
    ${APP_ICON_RESOURCE_WINDOWS}
#    ${CMAKE_CURRENT_SOURCE_DIR}/translations.qrc
    ${QM_FILES}
    ${TRANSLATIONS})
target_link_libraries(${PROJECT_NAME} PUBLIC Qt5::Core Qt5::Widgets Qt5::Sql Qt5::PrintSupport Qt5::Test )

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    )

install(FILES ${TRANSLATIONS} DESTINATION bin/translations)
