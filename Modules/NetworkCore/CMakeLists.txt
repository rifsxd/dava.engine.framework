set( MODULE_NAME NetworkCore )
set( MODULE_TYPE STATIC ) # INLINE, STATIC, DYNAMIC

set( HPP_FILES_RECURSE Sources/*.h )
set( CPP_FILES_RECURSE Sources/*.cpp )

set( INCLUDES Sources )
set( INCLUDES_PRIVATE ${DAVA_ENGINE_DIR}
                      ${DAVA_INCLUDE_DIR} )

set( MODULE_INITIALIZATION true )

setup_main_module()
