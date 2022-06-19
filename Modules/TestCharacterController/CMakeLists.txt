set( MODULE_NAME TestCharacterController)

set( MODULE_TYPE STATIC )

set( HPP_FILES_RECURSE *.h *.hpp )
set( CPP_FILES_RECURSE *.cpp *.mm )

set( INCLUDES  Sources )

set( INCLUDES_PRIVATE ${DAVA_INCLUDE_DIR} )

set( MIX_APP_DATA "Data = ${CMAKE_CURRENT_LIST_DIR}/Data" )

set( FIND_MODULE Physics )

set( MODULE_INITIALIZATION true )

setup_main_module()
