set( MODULE_NAME MemoryProfilerService )

set( MODULE_TYPE STATIC )

set( HPP_FILES_RECURSE *.h *.hpp )
set( CPP_FILES_RECURSE *.cpp *.mm )

set( INCLUDES Sources )

set( INCLUDES_PRIVATE ${DAVA_ENGINE_DIR}
                      ${DAVA_INCLUDE_DIR} )

set( FIND_MODULE NetworkHelpers )

setup_main_module()