macro( generated_unity_sources SOURCE_FILES )  

    if( UNITY_BUILD )
        message(STATUS ">>> Unity packages ${PROJECT_NAME} info" )

        #"find python"
        find_package( PythonInterp   )

        if( NOT PYTHONINTERP_FOUND )
            set( PYTHON_EXECUTABLE python )
        endif()

        #"remove duplicates"
        list ( LENGTH ${SOURCE_FILES} SOURCE_FILES_LIST_LEN )
        if ( ${SOURCE_FILES_LIST_LEN} GREATER "0" )
           list( REMOVE_DUPLICATES ${SOURCE_FILES} )
        endif()


        #"ARG"
        set( CUSTOM_PACK_MAX_NUMBER 20 )        
        set( ARG_PARAM "IGNORE_LIST;IGNORE_LIST_WIN32;IGNORE_LIST_APPLE;IGNORE_LIST_IOS;IGNORE_LIST_MACOS;IGNORE_LIST_ANDROID;UNITYIGNORE_FILES"  )
        foreach( index RANGE 1 ${CUSTOM_PACK_MAX_NUMBER} )
            set( ARG_PARAM "${ARG_PARAM};CUSTOM_PACK_${index}")
        endforeach()
        cmake_parse_arguments (ARG "" "" "${ARG_PARAM}" ${ARGN})

        #"ARG_IGNORE"
        foreach( TYPE_OS  APPLE IOS MACOS WIN32 ANDROID )
            if( ${TYPE_OS} )
                list( APPEND ARG_IGNORE_LIST ${ARG_IGNORE_LIST_${TYPE_OS}} )
            endif()
        endforeach()
        
        #"ARG_CUSTOM_PACK_${index}"
        foreach( index RANGE 1 ${CUSTOM_PACK_MAX_NUMBER} )
            if( ARG_CUSTOM_PACK_${index} )
                set( CUSTOM_PACK_${index}_PACK_EXP cpp ) 
                list(APPEND CUSTOM_PACKS CUSTOM_PACK_${index} )
            endif()
        endforeach()

        #"ARG_UNITYIGNORE_FILES"
        if(EXISTS ${GLOBAL_UNITYIGNORE_FILES})
            list( APPEND ARG_UNITYIGNORE_FILES ${GLOBAL_UNITYIGNORE_FILES} )
        endif()

        foreach( IGNORE_FILE  ${ARG_UNITYIGNORE_FILES} )
            message( "LOAD UNITY IGNORE FILE  - ${IGNORE_FILE}")

            file( STRINGS ${IGNORE_FILE} IGNORE_LIST )

            foreach( ITEM ${IGNORE_LIST})
                string( STRIP "${ITEM}" ITEM)

                if( NOT ITEM )
                    continue()
                endif()

                string( REGEX REPLACE "^;+" "" ITEM "${ITEM}" )
                string( REGEX REPLACE ";+$" "" ITEM "${ITEM}" )                
                STRING ( SUBSTRING "${ITEM}" 0 1 FIRST_SYMBOL )

                if( NOT (FIRST_SYMBOL MATCHES "#") )
                    string( REGEX REPLACE "[ ]+" ";" ITEM "${ITEM}" )
                    list( APPEND ARG_IGNORE_LIST ${ITEM} )
                    message("IGNORE MASK - ${ITEM}")
                else()
                    message("COMMENT - ${ITEM}")
                endif()
            endforeach()
            message("-")
        endforeach()

        if( ARG_IGNORE_LIST )
            list( REMOVE_DUPLICATES ARG_IGNORE_LIST ) 
        endif()

        #"BASE VALUES"
        set( IGNORE_LIST_SIZE 0   )
        set( REMAINING_LIST       )        
        set( OBJCPP_PACK_EXP mm   ) 
        set( CPP_PACK_EXP    cpp  )  
        foreach( PTYPE CPP OBJCPP ${CUSTOM_PACKS} )
            set( ${PTYPE}_PACK_SIZE 0      )
            set( ${PTYPE}_PACK_LIST        )
            set( ${PTYPE}_LIST_SIZE 0      )
            set( ${PTYPE}_LIST             )
            set( ${PTYPE}_ALL_LIST         )
            set( ${PTYPE}_ALL_LIST_SIZE    )
        endforeach()

        #"Formation of logical packs"
        foreach( ITEM ${${SOURCE_FILES}} )
            set( IGNORE_FLAG )
            set( PACK_FLAG )

            #"Put in the ignore list"
            foreach( IGNORE_MASK ${ARG_IGNORE_LIST} )
                if( ${ITEM} MATCHES ${IGNORE_MASK} )
                    set( IGNORE_FLAG true )
                    math( EXPR IGNORE_LIST_SIZE "${IGNORE_LIST_SIZE} + 1" )
                    break()
                endif()
            endforeach()

            #"Ext"
            string(REGEX MATCH "\\.[^.]+$"  ITEM_EXT ${ITEM})

            #"Put in custom packs"
            foreach( index RANGE 1 ${CUSTOM_PACK_MAX_NUMBER} )
                if( ARG_CUSTOM_PACK_${index} AND NOT IGNORE_FLAG )
                    foreach( IGNORE_MASK ${ARG_CUSTOM_PACK_${index}} )
                        if(     ${ITEM} MATCHES ${IGNORE_MASK} 
                            AND (    "${ITEM_EXT}" STREQUAL ".m" 
                                  OR "${ITEM_EXT}" STREQUAL ".mm"
                                  OR "${ITEM_EXT}" STREQUAL ".cpp"  
                                )
                           )
                            set( PACK_FLAG true )
                            list( APPEND CUSTOM_PACK_${index}_ALL_LIST  ${ITEM} )
                            break()
                        endif()
                    endforeach()
                endif()
            endforeach()
            
            #"Put in remaining lists"
            if( NOT PACK_FLAG )
                if( NOT IGNORE_FLAG AND "${ITEM_EXT}" STREQUAL ".cpp" )
                    list( APPEND CPP_ALL_LIST  ${ITEM} )
                elseif( NOT IGNORE_FLAG AND ( "${ITEM_EXT}" STREQUAL ".m" OR "${ITEM_EXT}" STREQUAL ".mm" ) )
                    list( APPEND OBJCPP_ALL_LIST  ${ITEM} )                
                else()
                    list( APPEND REMAINING_LIST ${ITEM} )
                endif()
            endif()

        endforeach()  
        
        #"Calculate the size of all the packs"
        foreach( PTYPE CPP OBJCPP ${CUSTOM_PACKS} )
            list( LENGTH ${PTYPE}_ALL_LIST  ${PTYPE}_ALL_LIST_SIZE )
        endforeach()

        if( NOT UNITY_BUILD_PACKAGES_NUMBER )
            set( UNITY_BUILD_PACKAGES_NUMBER 7 )
        endif()

        if( OBJCPP_ALL_LIST )
            math( EXPR UNITY_BUILD_PACKAGES_NUMBER "${UNITY_BUILD_PACKAGES_NUMBER} - 1" )
        endif()

        list( LENGTH   CUSTOM_PACKS CUSTOM_PACKS_SIZE )

        if( ${CUSTOM_PACKS_SIZE} GREATER 0 )
            if(    ${CUSTOM_PACKS_SIZE} GREATER ${UNITY_BUILD_PACKAGES_NUMBER} 
                OR ${CUSTOM_PACKS_SIZE} EQUAL   ${UNITY_BUILD_PACKAGES_NUMBER} )
                set(UNITY_BUILD_PACKAGES_NUMBER  1 )
            else()
                math( EXPR UNITY_BUILD_PACKAGES_NUMBER "${UNITY_BUILD_PACKAGES_NUMBER} - ${CUSTOM_PACKS_SIZE}" )
            endif()
        endif()   

        #"Generated pack files"
        foreach( PTYPE CPP OBJCPP ${CUSTOM_PACKS} )
            if( ${${PTYPE}_ALL_LIST_SIZE} EQUAL 0 )
                continue()
            endif()

            if( ${PTYPE} STREQUAL "CPP" )
                math( EXPR CPP_NUMBER_FILES_IN_PACK "${CPP_ALL_LIST_SIZE} / ${UNITY_BUILD_PACKAGES_NUMBER}" ) 
            else()
                math( EXPR ${PTYPE}_NUMBER_FILES_IN_PACK "${${PTYPE}_ALL_LIST_SIZE}" ) 
            endif( )

            foreach( ITEM ${${PTYPE}_ALL_LIST} )
                list( APPEND ${PTYPE}_LIST  ${ITEM} )
                math( EXPR ${PTYPE}_LIST_SIZE "${${PTYPE}_LIST_SIZE} + 1" )
                if( ${${PTYPE}_LIST_SIZE} GREATER ${${PTYPE}_NUMBER_FILES_IN_PACK} )
                    math( EXPR ${PTYPE}_PACK_SIZE "${${PTYPE}_PACK_SIZE} + 1" )
                    set( ${PTYPE}_PACK_${${PTYPE}_PACK_SIZE} ${${PTYPE}_LIST} )
                    set( ${PTYPE}_LIST )
                    set( ${PTYPE}_LIST_SIZE 0 )                
                endif()
            endforeach()  

            if( ${PTYPE}_LIST_SIZE )
                math( EXPR ${PTYPE}_PACK_SIZE "${${PTYPE}_PACK_SIZE} + 1" )
                set( ${PTYPE}_PACK_${${PTYPE}_PACK_SIZE} ${${PTYPE}_LIST} )

            endif()

            get_property( PACK_IDX GLOBAL PROPERTY  ${PROJECT_NAME}_PACK_IDX  )

            if( NOT PACK_IDX )
                set( PACK_IDX 0 )
            endif()
        
            foreach( index RANGE 1 ${${PTYPE}_PACK_SIZE}  )
                math( EXPR index_pack "${index} + ${PACK_IDX}" )
                set ( ${PTYPE}_NAME ${CMAKE_CURRENT_BINARY_DIR}/unity_pack/${PROJECT_NAME}_${index_pack}_${PTYPE}.${${PTYPE}_PACK_EXP} )
            
                #"calculation files hash"
                EXECUTE_PROCESS(
                    COMMAND ${PYTHON_EXECUTABLE} "${DAVA_SCRIPTS_FILES_PATH}/file_tree_hash.py" ${${PTYPE}_PACK_${index}} "--file_mode"
                    OUTPUT_VARIABLE SOURCE_FILES_HASH
                )
                string(REPLACE "\n" "" SOURCE_FILES_HASH "${SOURCE_FILES_HASH}" )


                #"checking exist of the pack"
                set( UNITY_PACK_EXISTS )
                if( EXISTS ${${PTYPE}_NAME} )
                    file(STRINGS ${${PTYPE}_NAME} SOURCE_FILES_HASH_READ LIMIT_COUNT 1)
                    string(REGEX MATCH "[^- ]+$"  SOURCE_FILES_HASH_READ ${SOURCE_FILES_HASH_READ})
                    if( ${SOURCE_FILES_HASH} STREQUAL ${SOURCE_FILES_HASH_READ})
                        set( UNITY_PACK_EXISTS true )
                    endif()
                endif()

                set( HEADERS_LIST "//file hash - ${SOURCE_FILES_HASH}")

                foreach( PACH ${${PTYPE}_PACK_${index}} )
                    get_filename_component( PACH ${PACH} ABSOLUTE )
                    list( APPEND HEADERS_LIST "#include \"${PACH}\"" ) 
                    set_source_files_properties( ${PACH} PROPERTIES HEADER_FILE_ONLY TRUE )
                endforeach()

                if( NOT UNITY_PACK_EXISTS )
                    string(REPLACE ";" "\n" HEADERS_LIST "${HEADERS_LIST}" )            
                    file( WRITE ${${PTYPE}_NAME} ${HEADERS_LIST})
                    #message( "generated pack     - ${${PTYPE}_NAME}")
                endif()

                list( APPEND ${PTYPE}_PACK_LIST ${${PTYPE}_NAME} )
            endforeach()

            math( EXPR PACK_IDX "${PACK_IDX} + ${${PTYPE}_PACK_SIZE}" )
            set_property( GLOBAL PROPERTY ${PROJECT_NAME}_PACK_IDX "${PACK_IDX}" )
        endforeach() 
        
        foreach( PTYPE CPP OBJCPP ${CUSTOM_PACKS} )
            list( APPEND ${SOURCE_FILES} ${${PTYPE}_PACK_LIST} )
        endforeach()

        list( LENGTH REMAINING_LIST REMAINING_LIST_SIZE )

        foreach( PTYPE CPP OBJCPP ${CUSTOM_PACKS} )
            message(STATUS "    ${PTYPE}_PACK_SIZE            - ${${PTYPE}_PACK_SIZE}")                
            message(STATUS "    ${PTYPE}_NUMBER_FILES_IN_PACK - ${${PTYPE}_NUMBER_FILES_IN_PACK}")
        endforeach() 

        message(STATUS "    IGNORE_LIST_SIZE            - ${IGNORE_LIST_SIZE}")

    endif()
endmacro ()