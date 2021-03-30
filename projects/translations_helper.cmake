include(CMakeParseArguments)
function(create_and_update_translations _qm_files)
    set(options)
    set(oneValueArgs)
    set(multiValueArgs OPTIONS)

    # parse arguments
    cmake_parse_arguments(_LUPDATE "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})
    set(_lupdate_files ${_LUPDATE_UNPARSED_ARGUMENTS})
    set(_lupdate_options ${_LUPDATE_OPTIONS})

    set(_my_sources)
    set(_my_tsfiles)
    set(_convert_translations FALSE)
    set(_converted_tsfiles)
    foreach(_file ${_lupdate_files})
        if(${_file} MATCHES "LIB")
            set(_convert_translations TRUE)
        else()
            get_filename_component(_ext ${_file} EXT)
            get_filename_component(_abs_FILE ${_file} ABSOLUTE)
            if(_ext MATCHES "ts")
                list(APPEND _my_tsfiles ${_abs_FILE})
            else()
                list(APPEND _my_sources ${_abs_FILE})
            endif()
        endif()
    endforeach()

    # Run lupdate for all ts files
    foreach(_ts_file ${_my_tsfiles})
        if(_my_sources)
            get_filename_component(_ts_name ${_ts_file} NAME_WE)

            set(_lst_file_srcs)
            foreach(_lst_file_src ${_my_sources})
                set(_lst_file_srcs "${_lst_file_src}\n${_lst_file_srcs}")
            endforeach()

            get_directory_property(_inc_DIRS INCLUDE_DIRECTORIES)

            foreach(_pro_include ${_inc_DIRS})
                get_filename_component(_abs_include "${_pro_include}" ABSOLUTE)
                set(_lst_file_srcs "-I${_pro_include}\n${_lst_file_srcs}")
            endforeach()

            # write lupdate arguments into temporary file
            set(_ts_lst_file "${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/${_ts_name}_lst_file")
            file(WRITE ${_ts_lst_file} "${_lst_file_srcs}")
        endif()
        if(_convert_translations)
            set(_conv_ts_name conv_${_ts_name})
        else()
            set(_conv_ts_name ${_ts_name})
        endif()

        add_custom_target(${_conv_ts_name} ALL
            COMMAND ${Qt5_LUPDATE_EXECUTABLE} ${_lupdate_options} "@${_ts_lst_file}" -ts ${_ts_file} 1> nul 2> nul
            DEPENDS ${_my_sources}
            BYPRODUCTS ${_ts_file}
            VERBATIM)
message("_conv_ts_name --------------- " ${_conv_ts_name})
        if(_convert_translations)
            get_filename_component(_ts_path ${_ts_file} DIRECTORY)
            set (_converted_tsfile "${_ts_path}/${_conv_ts_name}.ts")
            
            add_custom_command(TARGET ${_conv_ts_name} POST_BUILD 
            COMMAND ${CMAKE_COMMAND} -DTS_FILE=${_ts_file} 
            -P ${CMAKE_CURRENT_LIST_DIR}/../projects/postprocess_translations.cmake
            BYPRODUCTS ${_converted_tsfile}
            )

            list (APPEND _converted_tsfiles ${_converted_tsfile})
        endif()

    endforeach()

    if(NOT _convert_translations)
        set(_converted_tsfiles ${_my_tsfiles})
    endif()

    # create qm translations
    qt5_add_translation(${_qm_files} ${_converted_tsfiles})

    # return qm translations
    set(${_qm_files} ${${_qm_files}} PARENT_SCOPE)

endfunction()

