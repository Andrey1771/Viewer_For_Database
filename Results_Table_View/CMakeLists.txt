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

#set_source_files_properties(${QM_FILES} PROPERTIES OUTPUT_LOCATION "translations")

include(${CMAKE_SOURCE_DIR}/projects/translations_helper.cmake)
#qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} english.ts russian.ts)

create_and_update_translations(TRANSLATIONS ${HEADERS} ${SOURCES} ${UIS} ${RESOURCE} ${LANGUAGES} OPTIONS -locations none -silent ${LUPDATE_ADDITIONAL_OPTION})

#configure_file(translations.qrc ${CMAKE_BINARY_DIR} COPYONLY)
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



#set(TS_FILES helloworld_en.ts helloworld_de.ts)
#set_source_files_properties(${TS_FILES} PROPERTIES OUTPUT_LOCATION "l10n")
#qt5_add_translation(qmFiles ${TS_FILES})
#qt5_create_translation(TS_FILES ${CMAKE_SOURCE_DIR} helloworld_en.ts helloworld_de.ts)
#qt5_add_translation(QM_FILES english.ts russian.tss)



## translations
#set(LANG_TS_RU ${PROJECT_NAME}_ru.ts)
#add_custom_target(${PROJECT_NAME}_translations ALL)
#if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${LANG_TS_RU}")
#        add_custom_command(TARGET ${PROJECT_NAME}_translations PRE_BUILD
#                                                COMMAND ${CMAKE_COMMAND} -E
#                                                copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/${LANG_TS_RU}"
#                                                "${CMAKE_CURRENT_BINARY_DIR}/${LANG_TS_RU}")
#endif()

#set(LANGUAGES
#    ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}_en.ts
#    ${CMAKE_CURRENT_BINARY_DIR}/${LANG_TS_RU})


#create_and_update_translations(TRANSLATIONS ${HEADERS} ${SOURCES} ${UIS} ${UICS} ${LANGUAGES}
#    OPTIONS -locations none -silent ${LUPDATE_ADDITIONAL_OPTION})


#add_library(${PROJECT_NAME} SHARED ${HEADERS} ${SOURCES} ${TRANSLATIONS})

#target_compile_options(${PROJECT_NAME} PRIVATE
#    -Wall
#)

#install(FILES ${TRANSLATIONS} DESTINATION bin/translations)


