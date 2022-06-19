set( MODULE_NAME LoggerService )

set( MODULE_TYPE STATIC )

set( HPP_FILES_RECURSE *.h *.hpp )
set( CPP_FILES_RECURSE *.cpp *.mm )

set( INCLUDES Sources )
set( INCLUDES_PRIVATE ${CMAKE_CURRENT_LIST_DIR} 
                      ${DAVA_INCLUDE_DIR} )

set( DEFINITIONS_PRIVATE_WIN      -D_CRT_SECURE_NO_WARNINGS )
set( DEFINITIONS_PRIVATE_WINUAP   -D_CRT_SECURE_NO_WARNINGS )

set( FIND_MODULE NetworkHelpers )

setup_main_module()