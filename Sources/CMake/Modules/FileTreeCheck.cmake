
macro ( file_tree_check ) 
    find_package( PythonInterp   )

    set( TARGET_FILE_TREE_FOUND false )

    if( NOT PYTHONINTERP_FOUND )
        set( PYTHON_EXECUTABLE python )
    endif()

    if( NOT IGNORE_FILE_TREE_CHECK AND NOT DEPLOY )
        set( TARGET_FILE_TREE_FOUND true )

        get_property( ALL_PROJECTS_FOLDERS GLOBAL PROPERTY ALL_PROJECTS_FOLDERS )
        get_property( ALL_TARGET_PROJECTS_LIST GLOBAL PROPERTY ALL_TARGET_PROJECTS_LIST )
        get_property( ALL_TARGET_MODULES_LIST GLOBAL PROPERTY ALL_TARGET_MODULES_LIST )

        set( PROJECTS_FOLDERS )
        foreach( FOLDER ${ALL_PROJECTS_FOLDERS} )
            if( NOT ${FOLDER} MATCHES ${CMAKE_BINARY_DIR} )
                list( APPEND PROJECTS_FOLDERS ${FOLDER} )
            endif()
        endforeach()

        string(REPLACE ";" " " folders "${PROJECTS_FOLDERS}" )
        string(REPLACE "\"" "" folders "${PROJECTS_FOLDERS}" )

        EXECUTE_PROCESS(
            COMMAND ${PYTHON_EXECUTABLE} "${DAVA_SCRIPTS_FILES_PATH}/file_tree_hash.py" ${folders}
            OUTPUT_VARIABLE FILE_TREE_HASH
        )

        string(REPLACE "\n" "" FILE_TREE_HASH ${FILE_TREE_HASH})

        add_custom_target ( FILE_TREE_CHECK ALL 
            COMMAND ${PYTHON_EXECUTABLE} ${DAVA_SCRIPTS_FILES_PATH}/versions_check.py ${CMAKE_COMMAND} ${CMAKE_BINARY_DIR} ${FILE_TREE_HASH} ${folders}
        )

        set_target_properties( FILE_TREE_CHECK PROPERTIES FOLDER ${DAVA_PREDEFINED_TARGETS_FOLDER} )       
        
        foreach( TARGET ${ALL_TARGET_MODULES_LIST} )
            add_dependencies(  ${TARGET} FILE_TREE_CHECK )
        endforeach()

        foreach( TARGET ${ALL_TARGET_PROJECTS_LIST} )
            add_dependencies(  ${TARGET} FILE_TREE_CHECK )
        endforeach()

    endif()    

endmacro( file_tree_check )
















