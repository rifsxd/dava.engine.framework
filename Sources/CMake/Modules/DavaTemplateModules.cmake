
set(  MAIN_MODULE_VALUES 
MODULE_NAME                            #
MODULE_NAME_STUB                       #
MODULE_TYPE                            #"[ INLINE STATIC PLUGIN  ]"
#
IMPL_MODULE
MODULE_MANAGER
#
SRC_FOLDERS             
ERASE_FOLDERS              
ERASE_FOLDERS_${DAVA_PLATFORM_CURRENT}   
#
CPP_FILES                  
HPP_FILES                  
CPP_FILES_${DAVA_PLATFORM_CURRENT}       
HPP_FILES_${DAVA_PLATFORM_CURRENT}
#
HPP_FILES_STUB
HPP_FILES_IMPL
HPP_FILES_STUB_${DAVA_PLATFORM_CURRENT} 
HPP_FILES_IMPL_${DAVA_PLATFORM_CURRENT}   
#
CPP_FILES_STUB
CPP_FILES_IMPL
CPP_FILES_STUB_${DAVA_PLATFORM_CURRENT} 
CPP_FILES_IMPL_${DAVA_PLATFORM_CURRENT}        
#
CPP_FILES_RECURSE            
HPP_FILES_RECURSE            
CPP_FILES_RECURSE_${DAVA_PLATFORM_CURRENT} 
HPP_FILES_RECURSE_${DAVA_PLATFORM_CURRENT} 
GROUP_SOURCE
#
HPP_FILES_RECURSE_STUB
HPP_FILES_RECURSE_IMPL
HPP_FILES_RECURSE_STUB_${DAVA_PLATFORM_CURRENT} 
HPP_FILES_RECURSE_IMPL_${DAVA_PLATFORM_CURRENT}
CPP_FILES_RECURSE_STUB
CPP_FILES_RECURSE_IMPL
CPP_FILES_RECURSE_STUB_${DAVA_PLATFORM_CURRENT} 
CPP_FILES_RECURSE_IMPL_${DAVA_PLATFORM_CURRENT}
#
CPP_FILES_EXECUTE
#
PLATFORMS_SUPPORTED
PLATFORMS_IGNORE    #ANDROID 
                    #ANDROID_X86 
                    #ANDROID_ARM 
                    #WINUAP
                    #WINUAP_WIN32 
                    #WINUAP_ARM 
                    #WINUAP_X64 
                    #IOS
                    #MACOS
                    #WIN
                    #LINUX
#   
ERASE_FILES                
ERASE_FILES_${DAVA_PLATFORM_CURRENT}     
ERASE_FILES_NOT_${DAVA_PLATFORM_CURRENT} 
#
UNITY_IGNORE_LIST             
UNITY_IGNORE_LIST_${DAVA_PLATFORM_CURRENT}
#
CUSTOM_PACK_1
CUSTOM_PACK_1_${DAVA_PLATFORM_CURRENT}
#
INCLUDES         
INCLUDES_PRIVATE 
INCLUDES_${DAVA_PLATFORM_CURRENT} 
INCLUDES_PRIVATE_${DAVA_PLATFORM_CURRENT} 
#
DEFINITIONS                
DEFINITIONS_PRIVATE             
DEFINITIONS_${DAVA_PLATFORM_CURRENT}     
DEFINITIONS_PRIVATE_${DAVA_PLATFORM_CURRENT}  
USE_PARENT_DEFINITIONS
#
STATIC_LIBRARIES_${DAVA_PLATFORM_CURRENT}           
STATIC_LIBRARIES_${DAVA_PLATFORM_CURRENT}_RELEASE   
STATIC_LIBRARIES_${DAVA_PLATFORM_CURRENT}_DEBUG     
#
DYNAMIC_LIBRARIES_${DAVA_PLATFORM_CURRENT}           
DYNAMIC_LIBRARIES_${DAVA_PLATFORM_CURRENT}_RELEASE              
DYNAMIC_LIBRARIES_${DAVA_PLATFORM_CURRENT}_DEBUG                
#
FIND_SYSTEM_LIBRARY                   
FIND_SYSTEM_LIBRARY_${DAVA_PLATFORM_CURRENT}
#
FIND_MODULE
FIND_MODULE_${DAVA_PLATFORM_CURRENT}
#
FIND_PACKAGE
FIND_PACKAGE_${DAVA_PLATFORM_CURRENT}
#
QT_UI_FILES
QT_RES_FILES
#
DEPLOY_TO_BIN
DEPLOY_TO_BIN_${DAVA_PLATFORM_CURRENT}
BINARY_WIN32_DIR_RELEASE
BINARY_WIN32_DIR_DEBUG
BINARY_WIN32_DIR_RELWITHDEB
BINARY_WIN64_DIR_RELEASE
BINARY_WIN64_DIR_DEBUG
BINARY_WIN64_DIR_RELWITHDEB
#
#MIX_APP_DATA                   ## we should not change value of this variable
#
JAR_FOLDERS_ANDROID
JAVA_FOLDERS_ANDROID
#
PLUGIN_OUT_DIR
PLUGIN_OUT_DIR_${DAVA_PLATFORM_CURRENT}
#
PLUGIN_RELATIVE_PATH_TO_FOLDER
PLUGIN_COPY_ADD_FILES 
#
DEBUG_POSTFIX
CHECKED_POSTFIX
PROFILE_POSTFIX
RELEASE_POSTFIX
)

macro(apply_default_value VAR DEFAULT_VALUE)
    if (NOT ${VAR})
       set(${VAR} ${DEFAULT_VALUE})
    endif()
endmacro()

#
set(  GLOBAL_PROPERTY_VALUES ${MAIN_MODULE_VALUES}  TARGET_MODULES_LIST 
                                                    QT_DEPLOY_LIST_VALUE 
                                                    QT_LINKAGE_LIST 
                                                    QT_LINKAGE_LIST_VALUE 
                                                    DEPENDENT_LIST
                                                    GROUP_SOURCE )
#
macro( reset_MAIN_MODULE_VALUES )
    foreach( VALUE ${GLOBAL_PROPERTY_VALUES} GLOBAL_DEFINITIONS PLUGIN_LIST )
        set( ${VALUE} )
        set_property( GLOBAL PROPERTY ${VALUE} ${${VALUE}} )
    endforeach()
endmacro()
#
macro( dump_module_log  )

    set( MODULES_LOG_FILE  ${CMAKE_BINARY_DIR}/MODULES_LOG.txt )

    get_property( MODULE_CACHE_LOG_LIST GLOBAL PROPERTY MODULE_CACHE_LOG_LIST )

    if( MODULE_CACHE_LOG_LIST )

        set( UNIQUE_COMPONENTS_NUMBER 0 )
        set( USED_UNIQUE_COMPONENTS_NUMBER 0 )

        list( SORT MODULE_CACHE_LOG_LIST )

        file(WRITE ${MODULES_LOG_FILE} "\n" )

        file( APPEND ${MODULES_LOG_FILE} "UNIQUE COMPONENTS LIST\n\n" )
    
        foreach( ITEM ${MODULE_CACHE_LOG_LIST} )
            get_property( CACHE_LOG_${ITEM}_MODULE_UNIQUE GLOBAL PROPERTY CACHE_LOG_${ITEM}_MODULE_UNIQUE )

            get_property( CACHE_LOG_${ITEM}_MODULE_CACHE GLOBAL PROPERTY CACHE_LOG_${ITEM}_MODULE_CACHE )
            get_property( CACHE_LOG_${ITEM}_MODULE_MD5   GLOBAL PROPERTY CACHE_LOG_${ITEM}_MODULE_MD5 )
            get_property( CACHE_LOG_${ITEM}_MODULE_USES_LIST GLOBAL PROPERTY CACHE_LOG_${ITEM}_MODULE_USES_LIST )

            list(LENGTH CACHE_LOG_${ITEM}_MODULE_USES_LIST CACHE_LOG_${ITEM}_MODULE_USES_LIST_LENGTH )

            if( ${CACHE_LOG_${ITEM}_MODULE_UNIQUE} )
                math( EXPR UNIQUE_COMPONENTS_NUMBER "${UNIQUE_COMPONENTS_NUMBER} + 1" )
                file( APPEND ${MODULES_LOG_FILE} "-> ${UNIQUE_COMPONENTS_NUMBER}\n" )
                file( APPEND ${MODULES_LOG_FILE} "    MODULE_NAME  - ${ITEM} [ ${CACHE_LOG_${ITEM}_MODULE_USES_LIST_LENGTH} ]\n" )
                file( APPEND ${MODULES_LOG_FILE} "    USES_LIST    - ${CACHE_LOG_${ITEM}_MODULE_USES_LIST}\n" )
                file( APPEND ${MODULES_LOG_FILE} "    MODULE_CACHE - ${CACHE_LOG_${ITEM}_MODULE_CACHE}\n" )                
                file( APPEND ${MODULES_LOG_FILE} "    MD5          - ${CACHE_LOG_${ITEM}_MODULE_MD5}\n" )
                file( APPEND ${MODULES_LOG_FILE} "\n" )
            endif()

        endforeach()

        file( APPEND ${MODULES_LOG_FILE} "\n" )

        file( APPEND ${MODULES_LOG_FILE} "USED UNIQUE COMPONENTS LIST\n\n" )

        foreach( ITEM ${MODULE_CACHE_LOG_LIST} )
            get_property( CACHE_LOG_${ITEM}_MODULE_UNIQUE GLOBAL PROPERTY CACHE_LOG_${ITEM}_MODULE_UNIQUE )

            get_property( CACHE_LOG_${ITEM}_MODULE_CACHE GLOBAL PROPERTY CACHE_LOG_${ITEM}_MODULE_CACHE )
            get_property( CACHE_LOG_${ITEM}_MODULE_MD5   GLOBAL PROPERTY CACHE_LOG_${ITEM}_MODULE_MD5 )
   
            if( NOT ${CACHE_LOG_${ITEM}_MODULE_UNIQUE} )                
                math( EXPR USED_UNIQUE_COMPONENTS_NUMBER "${USED_UNIQUE_COMPONENTS_NUMBER} + 1" )
                file( APPEND ${MODULES_LOG_FILE} "-> ${USED_UNIQUE_COMPONENTS_NUMBER}\n" )
                file( APPEND ${MODULES_LOG_FILE} "    MODULE_NAME  - ${ITEM}\n" )
                file( APPEND ${MODULES_LOG_FILE} "    MODULE_CACHE - ${CACHE_LOG_${ITEM}_MODULE_CACHE}\n" )
                file( APPEND ${MODULES_LOG_FILE} "    MD5          - ${CACHE_LOG_${ITEM}_MODULE_MD5}\n" )
                file( APPEND ${MODULES_LOG_FILE} "\n" )
            endif()

        endforeach()

        list(LENGTH MODULE_CACHE_LOG_LIST MODULE_CACHE_LOG_LIST_LENGTH )

        file( APPEND ${MODULES_LOG_FILE} "\n\n" )
        file( APPEND ${MODULES_LOG_FILE} "UNIQUE      - ${UNIQUE_COMPONENTS_NUMBER}\n" )
        file( APPEND ${MODULES_LOG_FILE} "USED_UNIQUE - ${USED_UNIQUE_COMPONENTS_NUMBER}\n" )
        file( APPEND ${MODULES_LOG_FILE} "LIST_LENGTH - ${MODULE_CACHE_LOG_LIST_LENGTH}\n" )

    endif()

endmacro()
#
macro( setup_main_module )

    if( NOT MODULE_TYPE )
        set( MODULE_TYPE INLINE )
    endif()

    set( ORIGINAL_MODULE_NAME ${MODULE_NAME} )

    if( NOT ( ${MODULE_TYPE} STREQUAL "INLINE" ) )
        get_property( MODULES_ARRAY GLOBAL PROPERTY MODULES_ARRAY )
        list (FIND MODULES_ARRAY ${MODULE_NAME} _index)
        if ( JOIN_PROJECT_NAME OR ${_index} GREATER -1)
            set( MODULE_NAME ${MODULE_NAME}_${PROJECT_NAME} )
        endif() 
        list( APPEND MODULES_ARRAY ${MODULE_NAME} )
        set_property( GLOBAL PROPERTY MODULES_ARRAY "${MODULES_ARRAY}" )

        project ( ${MODULE_NAME} )
        include ( CMake-common )
    endif()

    set( INIT )

    get_filename_component (DIR_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)

    set( MODULE_COMPONENTS )

    if( MODULE_COMPONENTS_VALUE_NAME )
        get_property(  MODULE_COMPONENTS GLOBAL PROPERTY COMPONENTS_${MODULE_COMPONENTS_VALUE_NAME} )
        if( ORIGINAL_MODULE_NAME )
            list (FIND MODULE_COMPONENTS ${ORIGINAL_MODULE_NAME} _index)
            if ( ${_index} GREATER -1)
                set( INIT true )
                list( REMOVE_ITEM MODULE_COMPONENTS ${ORIGINAL_MODULE_NAME} )
            endif()
        else()
            set( INIT true )
        endif()

    else()
        set( INIT true )
    endif()

###

    if( PLATFORMS_IGNORE AND INIT )
        foreach( PLATFORM ${PLATFORMS_IGNORE} )
            if(${PLATFORM} STREQUAL ${DAVA_PLATFORM_CURRENT} )
                set( INIT )
                break()
            endif()
            list (FIND DAVA_PLATFORM_CURRENT_POSTFIXES ${PLATFORM} _index)
            if (${_index} GREATER -1)
                set( INIT )
                break()
            endif()
        endforeach()
    elseif( PLATFORMS_SUPPORTED AND INIT ) 
        set( INIT )
        foreach( PLATFORM ${PLATFORMS_SUPPORTED} )
            if(${PLATFORM} STREQUAL ${DAVA_PLATFORM_CURRENT} )
                set( INIT true )
                break()
            endif()
            list (FIND DAVA_PLATFORM_CURRENT_POSTFIXES ${PLATFORM} _index)
            if (${_index} GREATER -1)
                set( INIT true )
                break()
            endif()
        endforeach()
    endif()       

###
    if ( INIT )

        #"find root call"
        get_property( MAIN_MODULES_FIND_FIRST_CALL_LIST GLOBAL PROPERTY MAIN_MODULES_FIND_FIRST_CALL_LIST )
        if( NOT MAIN_MODULES_FIND_FIRST_CALL_LIST )  
            # "root call" 
            set( MODULE_STACK_DEFINITION )
        else()
            set( MODULE_STACK_DEFINITION ${MODULE_STACK_DEFINITION} ${DEFINITIONS} ${DEFINITIONS_${DAVA_PLATFORM_CURRENT}} PARENT_SCOPE)
        endif()

        list( APPEND MAIN_MODULES_FIND_FIRST_CALL_LIST "call" )
        set_property(GLOBAL PROPERTY MAIN_MODULES_FIND_FIRST_CALL_LIST ${MAIN_MODULES_FIND_FIRST_CALL_LIST} ) 
    endif()

    if ( INIT )
        
        if (${MODULE_TYPE} STREQUAL "PLUGIN"  )
            set( USE_PARENT_DEFINITIONS true )
        endif()


#####

        if(  NOT USE_PARENT_DEFINITIONS  )
            save_property( PROPERTY_LIST 
                DEFINITIONS
                DEFINITIONS_${DAVA_PLATFORM_CURRENT} )

        else()

            save_property( PROPERTY_LIST 
                DEFINITIONS
                DEFINITIONS_${DAVA_PLATFORM_CURRENT} )

            load_property( PROPERTY_LIST 
                DEFINITIONS
                DEFINITIONS_${DAVA_PLATFORM_CURRENT}
                GLOBAL_DEFINITIONS  ) 

        endif()


        #"FIND LIBRARY"
        foreach( NAME ${FIND_SYSTEM_LIBRARY} ${FIND_SYSTEM_LIBRARY_${DAVA_PLATFORM_CURRENT}} )
            FIND_LIBRARY( ${NAME}_LIBRARY  ${NAME} )

            if( ${NAME}_LIBRARY )
                list ( APPEND STATIC_LIBRARIES_SYSTEM_${DAVA_PLATFORM_CURRENT} ${${NAME}_LIBRARY} )
            else()
                if ( NOT NOT_TARGET_EXECUTABLE )
                    find_package( ${NAME} )
                    list ( APPEND STATIC_LIBRARIES_SYSTEM_${DAVA_PLATFORM_CURRENT} ${${NAME}_LIBRARY} )
                endif()
            endif()
        endforeach()

        #"FIND_MODULE"
        foreach( NAME ${FIND_MODULE} ${FIND_MODULE_${DAVA_PLATFORM_CURRENT}} )
            find_dava_module( ${NAME} COMPONENTS ${MODULE_COMPONENTS_${NAME}} )
        endforeach()

        #"FIND PACKAGE"
        foreach( NAME ${FIND_PACKAGE} ${FIND_PACKAGE${DAVA_PLATFORM_CURRENT}} )
            find_package( ${NAME} COMPONENTS ${MODULE_COMPONENTS} ${PACKAGE_COMPONENTS_${NAME}} )

            if (PACKAGE_${NAME}_INCLUDES)
                foreach( PACKAGE_INCLUDE ${PACKAGE_${NAME}_INCLUDES} )
                    include_directories(${${PACKAGE_INCLUDE}})
                endforeach()
            endif()
            list ( APPEND STATIC_LIBRARIES_SYSTEM_${DAVA_PLATFORM_CURRENT} ${PACKAGE_${NAME}_STATIC_LIBRARIES} )
        endforeach()

        list( APPEND DEFINITIONS ${MODULE_STACK_DEFINITION} )
        
#####
        if( CPP_FILES_EXECUTE )
            get_filename_component( CPP_FILES_EXECUTE ${CPP_FILES_EXECUTE} ABSOLUTE )
            save_property( PROPERTY_LIST CPP_FILES_EXECUTE )
        endif()        

#####
        if (${MODULE_TYPE} STREQUAL "STATIC"  )
            append_property(EXTERNAL_TEST_FOLDERS ${CMAKE_CURRENT_LIST_DIR})

            if( COVERAGE AND MACOS )
                set( COVERAGE_STRING "COVERAGE" )
            else()
                set( COVERAGE_STRING  )                
            endif()

            set( MODULE_CACHE   ${COVERAGE_STRING}
                                ${MODULE_COMPONENTS}  )
            
    
            list( APPEND MODULE_CACHE ${DEFINITIONS} 
                                      ${DEFINITIONS_${DAVA_PLATFORM_CURRENT}} 
                                    )
 

            if( MODULE_CACHE )
                list( REMOVE_DUPLICATES MODULE_CACHE )
                list( SORT MODULE_CACHE )
            endif()
            
            set( MODULE_CACHE   "ROOT_${ORIGINAL_MODULE_NAME}" ${MODULE_CACHE} )

            append_property( MODULE_CACHE_LOG_LIST ${MODULE_NAME}  )

            set_property( GLOBAL PROPERTY CACHE_LOG_${MODULE_NAME}_MODULE_CACHE  ${MODULE_CACHE} )

            string (REPLACE ";" " " MODULE_CACHE "${MODULE_CACHE}")
            string( MD5  MODULE_CACHE ${MODULE_CACHE} )

            set_property( GLOBAL PROPERTY CACHE_LOG_${MODULE_NAME}_MODULE_MD5  ${MODULE_CACHE}  )
            set_property( GLOBAL PROPERTY CACHE_LOG_${MODULE_NAME}_MODULE_UNIQUE  true )
            set_property( GLOBAL PROPERTY CACHE_LOG_${MODULE_NAME}_MODULE_USES_LIST  )

        endif()
#####            

        #"APPLE VALUES"
        if( APPLE )
            foreach( VALUE CPP_FILES 
                           CPP_FILES_RECURSE 
                           ERASE_FILES 
                           ERASE_FILES_NOT
                           DEFINITIONS 
                           DEFINITIONS_PRIVATE 
                           INCLUDES
                           INCLUDES_PRIVATE 
                           UNITY_IGNORE_LIST
                           CUSTOM_PACK_1 )
                if( ${VALUE}_APPLE)
                    list( APPEND ${VALUE}_${DAVA_PLATFORM_CURRENT} ${${VALUE}_APPLE} )  
                endif()
            endforeach()
        endif()

        #"INCLUDES"
        set( INCLUDES_LIST )
        foreach( ITEM ${INCLUDES} ${INCLUDES_${DAVA_PLATFORM_CURRENT}} )
            get_filename_component( ITEM ${ITEM} ABSOLUTE )
            list( APPEND INCLUDES_LIST ${ITEM} )
        endforeach()
        set( INCLUDES  ${INCLUDES_LIST} )
        list( APPEND INCLUDES_PRIVATE  ${INCLUDES_PRIVATE_${DAVA_PLATFORM_CURRENT}} )
        
        if( WIN )
            list( APPEND STATIC_LIBRARIES_WIN          ${STATIC_LIBRARIES_WIN${DAVA_PROJECT_BIT}} )
            list( APPEND STATIC_LIBRARIES_WIN_RELEASE  ${STATIC_LIBRARIES_WIN${DAVA_PROJECT_BIT}_RELEASE} ) 
            list( APPEND STATIC_LIBRARIES_WIN_DEBUG    ${STATIC_LIBRARIES_WIN${DAVA_PROJECT_BIT}_DEBUG} )
            list( APPEND DYNAMIC_LIBRARIES_WIN         ${DYNAMIC_LIBRARIES_WIN${DAVA_PROJECT_BIT}} )
            list( APPEND DYNAMIC_LIBRARIES_WIN_RELEASE ${DYNAMIC_LIBRARIES_WIN${DAVA_PROJECT_BIT}_RELEASE} )
            list( APPEND DYNAMIC_LIBRARIES_WIN_DEBUG   ${DYNAMIC_LIBRARIES_WIN${DAVA_PROJECT_BIT}_DEBUG} )

            foreach( CONFIGURE "_RELEASE" "_DEBUG" )
                foreach( DYNAMIC_LIBRARY ${DYNAMIC_LIBRARIES_WIN${DAVA_PROJECT_BIT}${CONFIGURE}} )
                    get_filename_component( DYNAMIC_LIBRARY ${DYNAMIC_LIBRARY} ABSOLUTE )
                    get_filename_component( DYNAMIC_LIBRARY_DIR ${DYNAMIC_LIBRARY}  DIRECTORY )
                    append_property( MODULE_DYNAMIC_LIBRARIES_DIR${CONFIGURE} ${DYNAMIC_LIBRARY_DIR} )
                endforeach()
            endforeach()

            foreach( DYNAMIC_LIBRARY ${DYNAMIC_LIBRARIES_WIN${DAVA_PROJECT_BIT}}  )
                get_filename_component( DYNAMIC_LIBRARY ${DYNAMIC_LIBRARY} ABSOLUTE )
                get_filename_component( DYNAMIC_LIBRARY_DIR ${DYNAMIC_LIBRARY}  DIRECTORY )
                append_property( MODULE_DYNAMIC_LIBRARIES_DIR ${DYNAMIC_LIBRARY_DIR} )
            endforeach()

        endif()      
       
    
        #"ERASE FILES"
        foreach( PLATFORM  ${DAVA_PLATFORM_LIST} )
            if( NOT ${PLATFORM} AND ERASE_FILES_NOT_${PLATFORM} )
                list( APPEND ERASE_FILES ${ERASE_FILES_NOT_${PLATFORM}} ) 
            endif()
        endforeach()
        if( ERASE_FILES_NOT_${DAVA_PLATFORM_CURRENT} AND ERASE_FILES )
             list(REMOVE_ITEM ERASE_FILES ${ERASE_FILES_NOT_${DAVA_PLATFORM_CURRENT}} )
        endif()

        set( ALL_SRC )
        set( ALL_SRC_HEADER_FILE_ONLY )
        
        if( SRC_FOLDERS  )

            foreach( VALUE ${MAIN_MODULE_VALUES} )
                set( ${VALUE}_DIR_NAME ${${VALUE}} )
                set( ${VALUE})
            endforeach()


            
            if( SRC_FOLDERS_DIR_NAME )
                define_source( SOURCE           ${SRC_FOLDERS_DIR_NAME}

                               IGNORE_ITEMS     ${CPP_FILES_EXECUTE} 
                                                ${ERASE_FOLDERS_DIR_NAME} 
                                                ${ERASE_FOLDERS_${DAVA_PLATFORM_CURRENT}_DIR_NAME} 
                                                ${ERASE_FILES_DIR_NAME} 
                                                ${ERASE_FILES_${DAVA_PLATFORM_CURRENT}_DIR_NAME}

                                GROUP_SOURCE    ${GROUP_SOURCE}
                            )
                                         
                set_project_files_properties( "${PROJECT_SOURCE_FILES_CPP}" )
                list( APPEND ALL_SRC  ${PROJECT_SOURCE_FILES} )
                list( APPEND ALL_SRC_HEADER_FILE_ONLY  ${PROJECT_HEADER_FILE_ONLY} )
            endif()
 
            foreach( VALUE ${MAIN_MODULE_VALUES} )
                set(  ${VALUE} ${${VALUE}_DIR_NAME} )
            endforeach()

        endif()

        if( MODULE_NAME_STUB )
            set( CONECTION_TYPE STUB )
            list (FIND MODULE_COMPONENTS ${MODULE_NAME_STUB} _index)

            if ( ${_index} GREATER -1 )
                set( MODULE_NAME )
                set( MODULE_TYPE INLINE )
                set( CONECTION_TYPE IMPL )
                add_module_subdirectory( ${MODULE_NAME_STUB}  "${IMPL_MODULE}" )

            endif()

            foreach ( ITEM  HPP_FILES_RECURSE HPP_FILES
                            CPP_FILES_RECURSE CPP_FILES )
                list( APPEND ${ITEM}   ${${ITEM}_${CONECTION_TYPE}} )
                list( APPEND ${ITEM}_${DAVA_PLATFORM_CURRENT} ${${ITEM}_${CONECTION_TYPE}_${DAVA_PLATFORM_CURRENT}} )
            endforeach ()

        endif()

        if (QT_UI_FILES OR QT_RES_FILES)
            file              ( GLOB_RECURSE UI_LIST  ${QT_UI_FILES})
            qt5_wrap_ui ( QT_UI_HEADERS ${UI_LIST} )

            file              ( GLOB_RECURSE RCC_LIST  ${QT_RES_FILES})
            qt5_add_resources ( QT_RCC  ${RCC_LIST} )

            list(APPEND HPP_FILES ${QT_UI_HEADERS})
            list(APPEND CPP_FILES ${QT_RCC})

            set(QtGenerated ${QT_UI_HEADERS} ${QT_RCC})
            list(APPEND GROUP_SOURCE QtGenerated)
        endif()

        define_source( SOURCE         ${CPP_FILES} ${CPP_FILES_${DAVA_PLATFORM_CURRENT}}
                                      ${HPP_FILES} ${HPP_FILES_${DAVA_PLATFORM_CURRENT}}
                       SOURCE_RECURSE ${CPP_FILES_RECURSE} ${CPP_FILES_RECURSE_${DAVA_PLATFORM_CURRENT}}
                                      ${HPP_FILES_RECURSE} ${HPP_FILES_RECURSE_${DAVA_PLATFORM_CURRENT}}
                       IGNORE_ITEMS   ${ERASE_FILES} ${ERASE_FILES_${DAVA_PLATFORM_CURRENT}} ${CPP_FILES_EXECUTE}
                       GROUP_SOURCE ${GROUP_SOURCE}
                       GROUP_STRINGS  ${MODULE_GROUP_STRINGS}
                     )


        list( APPEND ALL_SRC  ${PROJECT_SOURCE_FILES} )
        list( APPEND ALL_SRC_HEADER_FILE_ONLY  ${PROJECT_HEADER_FILE_ONLY} )
        list( APPEND MIX_APP_DATA  ${MIX_APP_DATA_${DAVA_PLATFORM_CURRENT}} )

        set_project_files_properties( "${ALL_SRC}" )
        

        #"SAVE PROPERTY"
        save_property( PROPERTY_LIST 
                DYNAMIC_LIBRARIES_${DAVA_PLATFORM_CURRENT}          
                DYNAMIC_LIBRARIES_${DAVA_PLATFORM_CURRENT}_RELEASE
                DYNAMIC_LIBRARIES_${DAVA_PLATFORM_CURRENT}_DEBUG
                STATIC_LIBRARIES_${DAVA_PLATFORM_CURRENT} 
                STATIC_LIBRARIES_${DAVA_PLATFORM_CURRENT}_RELEASE 
                STATIC_LIBRARIES_${DAVA_PLATFORM_CURRENT}_DEBUG 
                STATIC_LIBRARIES_SYSTEM_${DAVA_PLATFORM_CURRENT}
                DEPLOY_TO_BIN
                DEPLOY_TO_BIN_${DAVA_PLATFORM_CURRENT}
                INCLUDES
                INCLUDES_PRIVATE
                BINARY_WIN32_DIR_RELEASE
                BINARY_WIN32_DIR_DEBUG
                BINARY_WIN32_DIR_RELWITHDEB
                BINARY_WIN64_DIR_RELEASE
                BINARY_WIN64_DIR_DEBUG
                BINARY_WIN64_DIR_RELWITHDEB
                JAR_FOLDERS_ANDROID
                JAVA_FOLDERS_ANDROID
                MIX_APP_DATA
                )

        load_property( PROPERTY_LIST                
                STATIC_LIBRARIES_${DAVA_PLATFORM_CURRENT} 
                STATIC_LIBRARIES_${DAVA_PLATFORM_CURRENT}_RELEASE 
                STATIC_LIBRARIES_${DAVA_PLATFORM_CURRENT}_DEBUG 
                STATIC_LIBRARIES_SYSTEM_${DAVA_PLATFORM_CURRENT}
                INCLUDES
                INCLUDES_PRIVATE
                PLATFORM_DEFINITIONS_${DAVA_PLATFORM_CURRENT}
                )

        if( ${MODULE_TYPE} STREQUAL "PLUGIN" )
            load_property( PROPERTY_LIST
                DYNAMIC_LIBRARIES_${DAVA_PLATFORM_CURRENT}          
                DYNAMIC_LIBRARIES_${DAVA_PLATFORM_CURRENT}_RELEASE
                DYNAMIC_LIBRARIES_${DAVA_PLATFORM_CURRENT}_DEBUG
                )
        endif()

        if(  NOT USE_PARENT_DEFINITIONS  )
            save_property( PROPERTY_LIST 
                DEFINITIONS
                DEFINITIONS_${DAVA_PLATFORM_CURRENT} )

        else()

            save_property( PROPERTY_LIST 
                DEFINITIONS
                DEFINITIONS_${DAVA_PLATFORM_CURRENT} )

            load_property( PROPERTY_LIST 
                DEFINITIONS
                DEFINITIONS_${DAVA_PLATFORM_CURRENT}
                GLOBAL_DEFINITIONS  ) 

            list( APPEND DEFINITIONS ${GLOBAL_DEFINITIONS} )

        endif()
        
        list( APPEND DEFINITIONS ${PLATFORM_DEFINITIONS_${DAVA_PLATFORM_CURRENT}} )


        #"DEFINITIONS"
        if( DEFINITIONS )
            add_definitions( ${DEFINITIONS} )
        endif()
        if( DEFINITIONS_${DAVA_PLATFORM_CURRENT} )
            add_definitions( ${DEFINITIONS_${DAVA_PLATFORM_CURRENT}} )
        endif()

        #"INCLUDES_DIR"
        load_property( PROPERTY_LIST INCLUDES )
        if( INCLUDES )
            include_directories( "${INCLUDES}" )  
        endif()

        #"PLUGIN_OUT_DIR"
        if( PLUGIN_OUT_DIR_${DAVA_PLATFORM_CURRENT} )
            set( PLUGIN_OUT_DIR PLUGIN_OUT_DIR_${DAVA_PLATFORM_CURRENT}  )
        endif()

        if( ${MODULE_TYPE} STREQUAL "INLINE" )
            set (${DIR_NAME}_PROJECT_SOURCE_FILES_CPP ${PROJECT_SOURCE_FILES_CPP} PARENT_SCOPE)
            set (${DIR_NAME}_PROJECT_SOURCE_FILES_HPP ${PROJECT_SOURCE_FILES_HPP} PARENT_SCOPE)
        else()
#####
            set( CREATE_NEW_MODULE true )

            if( ${MODULE_TYPE} STREQUAL "STATIC" )
                get_property( MODULE_CACHE_LIST GLOBAL PROPERTY MODULE_CACHE_LIST )
                list (FIND MODULE_CACHE_LIST ${MODULE_CACHE} _index)
                if ( ${_index} GREATER -1 )
                    set( CREATE_NEW_MODULE )
                    list(GET MODULE_CACHE_LIST ${_index}  MODULE_CACHE )
                    get_property( MODULE_CACHE_LOADED_NAME GLOBAL PROPERTY ${MODULE_CACHE} )
                    set_property( GLOBAL PROPERTY CACHE_LOG_${MODULE_NAME}_MODULE_UNIQUE  false )
                    append_property( CACHE_LOG_${MODULE_CACHE_LOADED_NAME}_MODULE_USES_LIST ${MODULE_NAME} )  
                    set( MODULE_NAME ${MODULE_CACHE_LOADED_NAME} )
                endif()
            endif()

######

            if( CREATE_NEW_MODULE )
                project( ${MODULE_NAME} )
                
                generated_unity_sources( ALL_SRC  IGNORE_LIST ${UNITY_IGNORE_LIST}
                                                  IGNORE_LIST_${DAVA_PLATFORM_CURRENT} ${UNITY_IGNORE_LIST_${DAVA_PLATFORM_CURRENT}}
                                                  CUSTOM_PACK_1 ${CUSTOM_PACK_1} ${CUSTOM_PACK_1_${DAVA_PLATFORM_CURRENT}}
                                                  UNITYIGNORE_FILES ${PROJECT_UNITYIGNORE_FILES} ) 
            endif()

            if( ${MODULE_TYPE} STREQUAL "STATIC" )

                if( MODULE_MANAGER )
                    set_property( GLOBAL PROPERTY MODULE_MANAGER  true )
                    get_filename_component( MODULE_MANAGER_STUB ${MODULE_MANAGER_STUB} ABSOLUTE )
                    set_property( GLOBAL PROPERTY MODULE_MANAGER_STUB  ${MODULE_MANAGER_STUB} )
                endif()

                if( MODULE_INITIALIZATION )
                    append_property( DAVA_LOADED_INITIALIZATION_MODULES ${ORIGINAL_MODULE_NAME} )
                endif()

                if( CREATE_NEW_MODULE )
                    add_library( ${MODULE_NAME} STATIC  ${ALL_SRC} ${ALL_SRC_HEADER_FILE_ONLY} )
                endif()
                append_property( TARGET_MODULES_LIST ${MODULE_NAME} )  
                append_property( ALL_TARGET_MODULES_LIST ${MODULE_NAME} )  

            elseif( ${MODULE_TYPE} STREQUAL "PLUGIN" )

                get_property( MODULE_MANAGER_STUB GLOBAL PROPERTY MODULE_MANAGER_STUB )

                add_library( ${MODULE_NAME} SHARED  ${ALL_SRC} ${ALL_SRC_HEADER_FILE_ONLY} ${MODULE_MANAGER_STUB} )
                append_property( PLUGIN_LIST ${MODULE_NAME} )

                load_property( PROPERTY_LIST TARGET_MODULES_LIST ) 
                list( APPEND STATIC_LIBRARIES_${DAVA_PLATFORM_CURRENT} ${TARGET_MODULES_LIST} )  
                add_definitions( -DDAVA_IMPLEMENT_PLUGIN_MODULE )  

                if( WIN32 )
                    set_target_properties ( ${PROJECT_NAME} PROPERTIES LINK_FLAGS_RELEASE "/DEBUG" )
                     set_target_properties ( ${PROJECT_NAME} PROPERTIES LINK_FLAGS "/SAFESEH:NO" )

                    # Generate debug info also in release builds
                    set_target_properties ( ${PROJECT_NAME} PROPERTIES LINK_FLAGS_RELEASE "/DEBUG /SUBSYSTEM:WINDOWS" )
                    set_target_properties ( ${PROJECT_NAME} PROPERTIES LINK_FLAGS_RELWITHDEBINFO "/DEBUG /SUBSYSTEM:WINDOWS" )
                endif()

                apply_default_value(DEBUG_POSTFIX "Debug")
                apply_default_value(CHECKED_POSTFIX " ")
                apply_default_value(PROFILE_POSTFIX " ")
                apply_default_value(RELEASE_POSTFIX " ")

                set_target_properties( ${MODULE_NAME} PROPERTIES
                                                                 DEBUG_OUTPUT_NAME "${MODULE_NAME}" 
                                                                 DEBUG_POSTFIX ${DEBUG_POSTFIX}
                                                                 CHECKED_POSTFIX ${CHECKED_POSTFIX}
                                                                 PROFILE_POSTFIX ${PROFILE_POSTFIX}
                                                                 RELEASE_POSTFIX ${RELEASE_POSTFIX})

                if( WIN32 AND NOT DEPLOY )
                    set( BINARY_WIN32_DIR_RELEASE    "${BINARY_WIN32_DIR_RELEASE}" "${CMAKE_CURRENT_BINARY_DIR}/Release" )
                    set( BINARY_WIN32_DIR_DEBUG    "${BINARY_WIN32_DIR_DEBUG}"   "${CMAKE_CURRENT_BINARY_DIR}/Debug" )
                    set( BINARY_WIN32_DIR_RELWITHDEB  "${BINARY_WIN32_DIR_RELWITHDEB}"  "${CMAKE_CURRENT_BINARY_DIR}/RelWithDebinfo" )
                    set( BINARY_WIN64_DIR_RELEASE  "${BINARY_WIN64_DIR_RELEASE}"  "${CMAKE_CURRENT_BINARY_DIR}/Release" )
                    set( BINARY_WIN64_DIR_DEBUG    "${BINARY_WIN64_DIR_DEBUG}"  "${CMAKE_CURRENT_BINARY_DIR}/Debug" )
                    set( BINARY_WIN64_DIR_RELWITHDEB "${BINARY_WIN64_DIR_RELWITHDEB}" "${CMAKE_CURRENT_BINARY_DIR}/RelWithDebinfo" )
                    save_property( PROPERTY_LIST BINARY_WIN32_DIR_RELEASE 
                                                 BINARY_WIN32_DIR_DEBUG
                                                 BINARY_WIN32_DIR_RELWITHDEB
                                                 BINARY_WIN64_DIR_RELEASE 
                                                 BINARY_WIN64_DIR_DEBUG
                                                 BINARY_WIN64_DIR_RELWITHDEB )
                endif()

                if( MACOS )
                    list( APPEND STATIC_LIBRARIES_${DAVA_PLATFORM_CURRENT}  ${DYNAMIC_LIBRARIES_${DAVA_PLATFORM_CURRENT}})
                    list( APPEND STATIC_LIBRARIES_${DAVA_PLATFORM_CURRENT}_RELEASE  ${DYNAMIC_LIBRARIES_${DAVA_PLATFORM_CURRENT}_RELEASE})
                    list( APPEND STATIC_LIBRARIES_${DAVA_PLATFORM_CURRENT}_DEBUG  ${DYNAMIC_LIBRARIES_${DAVA_PLATFORM_CURRENT}_DEBUG})
                endif()

                if( PLUGIN_OUT_DIR )
                    foreach( OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES} )
                        string( TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG )
                        
                        if( APPLE )
                            set_target_properties( ${MODULE_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${PLUGIN_OUT_DIR} )                
                        else()
                            set_target_properties( ${MODULE_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${PLUGIN_OUT_DIR} )
                        endif()

                    endforeach( OUTPUTCONFIG CMAKE_CONFIGURATION_TYPES )
                endif()
                 
                if( PLUGIN_RELATIVE_PATH_TO_FOLDER )
                    set_property( GLOBAL PROPERTY ${MODULE_NAME}_RELATIVE_PATH_TO_FOLDER ${PLUGIN_RELATIVE_PATH_TO_FOLDER} )
                endif()

                if( PLUGIN_COPY_ADD_FILES )
                    set_property( GLOBAL PROPERTY ${MODULE_NAME}_PLUGIN_COPY_ADD_FILES ${PLUGIN_COPY_ADD_FILES} )                    
                endif()

            endif()

            if( DEFINITIONS_PRIVATE )
                add_definitions( ${DEFINITIONS_PRIVATE} )
            endif()

            if( DEFINITIONS_PRIVATE_${DAVA_PLATFORM_CURRENT} )
                add_definitions( ${DEFINITIONS_PRIVATE_${DAVA_PLATFORM_CURRENT}} )
            endif()

            if( INCLUDES_PRIVATE )
                include_directories( ${INCLUDES_PRIVATE} ) 
            endif() 


            if( CREATE_NEW_MODULE )

                if( WIN32 )
                    grab_libs(LIST_SHARED_LIBRARIES_DEBUG   "${STATIC_LIBRARIES_${DAVA_PLATFORM_CURRENT}_DEBUG}"   EXCLUDE_LIBS ADDITIONAL_DEBUG_LIBS)
                    grab_libs(LIST_SHARED_LIBRARIES_RELEASE "${STATIC_LIBRARIES_${DAVA_PLATFORM_CURRENT}_RELEASE}" EXCLUDE_LIBS ADDITIONAL_RELEASE_LIBS)
                    set( STATIC_LIBRARIES_${DAVA_PLATFORM_CURRENT}_DEBUG   ${LIST_SHARED_LIBRARIES_DEBUG} )
                    set( STATIC_LIBRARIES_${DAVA_PLATFORM_CURRENT}_RELEASE ${LIST_SHARED_LIBRARIES_RELEASE} )
                endif()

                if( LINK_THIRD_PARTY )                 
                    MERGE_STATIC_LIBRARIES( ${MODULE_NAME} ALL "${STATIC_LIBRARIES_${DAVA_PLATFORM_CURRENT}}" )
                    MERGE_STATIC_LIBRARIES( ${PROJECT_NAME} DEBUG "${STATIC_LIBRARIES_${DAVA_PLATFORM_CURRENT}_DEBUG}" )
                    MERGE_STATIC_LIBRARIES( ${PROJECT_NAME} RELEASE "${STATIC_LIBRARIES_${DAVA_PLATFORM_CURRENT}_RELEASE}" )
                endif()

                target_link_libraries  ( ${MODULE_NAME}  ${STATIC_LIBRARIES_${DAVA_PLATFORM_CURRENT}}
                                                         ${STATIC_LIBRARIES_SYSTEM_${DAVA_PLATFORM_CURRENT}} )  

                foreach ( FILE ${STATIC_LIBRARIES_${DAVA_PLATFORM_CURRENT}_DEBUG} )
                    target_link_libraries  ( ${MODULE_NAME} debug ${FILE} )
                endforeach ()

                foreach ( FILE ${STATIC_LIBRARIES_${DAVA_PLATFORM_CURRENT}_RELEASE} )
                    target_link_libraries  ( ${MODULE_NAME} optimized ${FILE} )
                endforeach ()

                list (FIND FIND_PACKAGE QT5 _index)
                if (NOT ${_index} MATCHES -1 )
                    link_with_qt5(${PROJECT_NAME})
                endif()

                if( COVERAGE AND MACOS )
              
                    string(REPLACE ";" " " TARGET_FOLDERS_${PROJECT_NAME} "${TARGET_FOLDERS_${PROJECT_NAME}}" )
                    string(REPLACE "\"" "" TARGET_FOLDERS_${PROJECT_NAME} "${TARGET_FOLDERS_${PROJECT_NAME}}" )

                    add_definitions( -DTEST_COVERAGE )
                    add_definitions( -DDAVA_FOLDERS="${DAVA_FOLDERS}" )
                    add_definitions( -DDAVA_UNITY_FOLDER="${CMAKE_BINARY_DIR}/unity_pack" )

                    list( APPEND EXECUTE_DEFINITIONS -DTARGET_FOLDERS_${ORIGINAL_MODULE_NAME}="${TARGET_FOLDERS_${PROJECT_NAME}}" )

                    append_property( EXECUTE_DEFINITIONS_${MODULE_NAME} "${EXECUTE_DEFINITIONS}" )

                    set_target_properties(${MODULE_NAME} PROPERTIES XCODE_ATTRIBUTE_GCC_GENERATE_TEST_COVERAGE_FILES YES )
                    set_target_properties(${MODULE_NAME} PROPERTIES XCODE_ATTRIBUTE_GCC_INSTRUMENT_PROGRAM_FLOW_ARCS YES )

                endif()   

                if ( WINDOWS_UAP )
                    set_property(TARGET ${MODULE_NAME} PROPERTY VS_MOBILE_EXTENSIONS_VERSION ${WINDOWS_UAP_MOBILE_EXT_SDK_VERSION} )
                endif()             
            endif()

            reset_property( STATIC_LIBRARIES_${DAVA_PLATFORM_CURRENT} )
            reset_property( STATIC_LIBRARIES_${DAVA_PLATFORM_CURRENT}_RELEASE )
            reset_property( STATIC_LIBRARIES_${DAVA_PLATFORM_CURRENT}_DEBUG )
            reset_property( STATIC_LIBRARIES_SYSTEM_${DAVA_PLATFORM_CURRENT} )
            reset_property( INCLUDES_PRIVATE )       
                     
        endif()

        #"find root call"
        get_property( MAIN_MODULES_FIND_FIRST_CALL_LIST GLOBAL PROPERTY MAIN_MODULES_FIND_FIRST_CALL_LIST )
        list( REMOVE_AT  MAIN_MODULES_FIND_FIRST_CALL_LIST 0 )
        set_property(GLOBAL PROPERTY MAIN_MODULES_FIND_FIRST_CALL_LIST ${MAIN_MODULES_FIND_FIRST_CALL_LIST} )        
        
        list( LENGTH MAIN_MODULES_FIND_FIRST_CALL_LIST LENGTH_DEFINE_SOURCE_LIST  )
        if ( NOT LENGTH_DEFINE_SOURCE_LIST )
            #"root call"
            set_property( GLOBAL PROPERTY MODULES_NAME "${MODULE_NAME}" )
        endif()

        if( CREATE_NEW_MODULE AND ${MODULE_TYPE} STREQUAL "STATIC" )
            set_property( GLOBAL PROPERTY ${MODULE_CACHE} "${MODULE_NAME}" )
            append_property(  MODULE_CACHE_LIST ${MODULE_CACHE} )
        endif()

    endif()

endmacro ()



